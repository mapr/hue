"""
Template for each `dtype` helper function for rank

WARNING: DO NOT edit .pxi FILE directly, .pxi is generated from .pxi.in
"""

#----------------------------------------------------------------------
# rank_1d, rank_2d
#----------------------------------------------------------------------


@cython.wraparound(False)
@cython.boundscheck(False)


def rank_1d_object(object in_arr, bint retry=1, ties_method='average',
                      ascending=True, na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, n, dups = 0, total_tie_count = 0

        ndarray sorted_data, values

        ndarray[float64_t] ranks
        ndarray[int64_t] argsorted

        object val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0
    tiebreak = tiebreakers[ties_method]

    values = np.array(in_arr, copy=True)

    if values.dtype != np.object_:
        values = values.astype('O')

    keep_na = na_option == 'keep'

    if ascending ^ (na_option == 'top'):
        nan_value = Infinity()
    else:
        nan_value = NegInfinity()

    mask = lib.isnaobj(values)

    np.putmask(values, mask, nan_value)

    n = len(values)
    ranks = np.empty(n, dtype='f8')

    try:
        _as = values.argsort()
    except TypeError:
        if not retry:
            raise

        valid_locs = (~mask).nonzero()[0]
        ranks.put(valid_locs, rank_1d_object(values.take(valid_locs), 0,
                                             ties_method=ties_method,
                                             ascending=ascending))
        np.putmask(ranks, mask, np.nan)
        return ranks

    if not ascending:
        _as = _as[::-1]

    sorted_data = values.take(_as)
    argsorted = _as.astype('i8')

    for i in range(n):
        sum_ranks += i + 1
        dups += 1

        val = util.get_value_at(sorted_data, i)

        if (val is nan_value) and keep_na:
            ranks[argsorted[i]] = nan
            continue

        count += 1.0

        if (i == n - 1 or
            are_diff(util.get_value_at(sorted_data, i + 1), val)):
            if tiebreak == TIEBREAK_AVERAGE:
                for j in range(i - dups + 1, i + 1):
                    ranks[argsorted[j]] = sum_ranks / dups
            elif tiebreak == TIEBREAK_MIN:
                for j in range(i - dups + 1, i + 1):
                    ranks[argsorted[j]] = i - dups + 2
            elif tiebreak == TIEBREAK_MAX:
                for j in range(i - dups + 1, i + 1):
                    ranks[argsorted[j]] = i + 1
            elif tiebreak == TIEBREAK_FIRST:
                raise ValueError('first not supported for non-numeric data')
            elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                for j in range(i - dups + 1, i + 1):
                    ranks[argsorted[j]] = 2 * i - j - dups + 2
            elif tiebreak == TIEBREAK_DENSE:
                total_tie_count += 1
                for j in range(i - dups + 1, i + 1):
                    ranks[argsorted[j]] = total_tie_count
            sum_ranks = dups = 0
    if pct:
        return ranks / count
    else:
        return ranks


def rank_2d_object(object in_arr, axis=0, ties_method='average',
                      ascending=True, na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, z, k, n, dups = 0, total_tie_count = 0

        Py_ssize_t infs

        ndarray[float64_t, ndim=2] ranks
        ndarray[object, ndim=2] values

        ndarray[int64_t, ndim=2] argsorted

        object val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0

    tiebreak = tiebreakers[ties_method]

    keep_na = na_option == 'keep'

    in_arr = np.asarray(in_arr)

    if axis == 0:
        values = in_arr.T.copy()
    else:
        values = in_arr.copy()

    if values.dtype != np.object_:
        values = values.astype('O')
    if ascending ^ (na_option == 'top'):
        nan_value = Infinity()
    else:
        nan_value = NegInfinity()

    mask = lib.isnaobj2d(values)

    np.putmask(values, mask, nan_value)

    n, k = (<object> values).shape
    ranks = np.empty((n, k), dtype='f8')

    try:
        _as = values.argsort(1)
    except TypeError:
        values = in_arr
        for i in range(len(values)):
            ranks[i] = rank_1d_object(in_arr[i], ties_method=ties_method,
                                      ascending=ascending, pct=pct)
        if axis == 0:
            return ranks.T
        else:
            return ranks

    if not ascending:
        _as = _as[:, ::-1]

    values = _take_2d_object(values, _as)
    argsorted = _as.astype('i8')

    for i in range(n):
        dups = sum_ranks = infs = 0

        total_tie_count = 0
        count = 0.0
        for j in range(k):

            val = values[i, j]

            if (val is nan_value) and keep_na:
                ranks[i, argsorted[i, j]] = nan

                infs += 1

                continue

            count += 1.0

            sum_ranks += (j - infs) + 1
            dups += 1
            if j == k - 1 or are_diff(values[i, j + 1], val):
                if tiebreak == TIEBREAK_AVERAGE:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j + 1
                elif tiebreak == TIEBREAK_FIRST:
                    raise ValueError('first not supported '
                                     'for non-numeric data')
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = 2 * j - z - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = total_tie_count
                sum_ranks = dups = 0
        if pct:
            ranks[i, :] /= count
    if axis == 0:
        return ranks.T
    else:
        return ranks


@cython.wraparound(False)
@cython.boundscheck(False)


def rank_1d_float64(object in_arr, ties_method='average', ascending=True,
                      na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, n, dups = 0, total_tie_count = 0

        ndarray[float64_t] sorted_data, values

        ndarray[float64_t] ranks
        ndarray[int64_t] argsorted

        float64_t val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0
    tiebreak = tiebreakers[ties_method]

    values = np.asarray(in_arr).copy()

    keep_na = na_option == 'keep'

    if ascending ^ (na_option == 'top'):
        nan_value = np.inf
    else:
        nan_value = -np.inf

    mask = np.isnan(values)

    np.putmask(values, mask, nan_value)

    n = len(values)
    ranks = np.empty(n, dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort()

    if not ascending:
        _as = _as[::-1]

    sorted_data = values.take(_as)
    argsorted = _as.astype('i8')

    with nogil:
        for i in range(n):
            sum_ranks += i + 1
            dups += 1

            val = sorted_data[i]

            if (val == nan_value) and keep_na:
                ranks[argsorted[i]] = nan
                continue

            count += 1.0

            if i == n - 1 or sorted_data[i + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = j + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = 2 * i - j - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = total_tie_count
                sum_ranks = dups = 0
    if pct:
        return ranks / count
    else:
        return ranks


def rank_2d_float64(object in_arr, axis=0, ties_method='average',
                      ascending=True, na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, z, k, n, dups = 0, total_tie_count = 0


        ndarray[float64_t, ndim=2] ranks
        ndarray[float64_t, ndim=2] values

        ndarray[int64_t, ndim=2] argsorted

        float64_t val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0

    tiebreak = tiebreakers[ties_method]

    keep_na = na_option == 'keep'

    in_arr = np.asarray(in_arr)

    if axis == 0:
        values = in_arr.T.copy()
    else:
        values = in_arr.copy()

    if ascending ^ (na_option == 'top'):
        nan_value = np.inf
    else:
        nan_value = -np.inf

    mask = np.isnan(values)

    np.putmask(values, mask, nan_value)

    n, k = (<object> values).shape
    ranks = np.empty((n, k), dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(axis=1, kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort(1)

    if not ascending:
        _as = _as[:, ::-1]

    values = _take_2d_float64(values, _as)
    argsorted = _as.astype('i8')

    for i in range(n):
        dups = sum_ranks = 0

        total_tie_count = 0
        count = 0.0
        for j in range(k):
            sum_ranks += j + 1
            dups += 1

            val = values[i, j]

            if (val == nan_value) and keep_na:
                ranks[i, argsorted[i, j]] = nan


                continue

            count += 1.0

            if j == k - 1 or values[i, j + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = z + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = 2 * j - z - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = total_tie_count
                sum_ranks = dups = 0
        if pct:
            ranks[i, :] /= count
    if axis == 0:
        return ranks.T
    else:
        return ranks


@cython.wraparound(False)
@cython.boundscheck(False)


def rank_1d_uint64(object in_arr, ties_method='average', ascending=True,
                      na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, n, dups = 0, total_tie_count = 0

        ndarray[uint64_t] sorted_data, values

        ndarray[float64_t] ranks
        ndarray[int64_t] argsorted

        uint64_t val

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0
    tiebreak = tiebreakers[ties_method]

    values = np.asarray(in_arr)

    keep_na = na_option == 'keep'


    n = len(values)
    ranks = np.empty(n, dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort()

    if not ascending:
        _as = _as[::-1]

    sorted_data = values.take(_as)
    argsorted = _as.astype('i8')

    with nogil:
        for i in range(n):
            sum_ranks += i + 1
            dups += 1

            val = sorted_data[i]


            count += 1.0

            if i == n - 1 or sorted_data[i + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = j + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = 2 * i - j - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = total_tie_count
                sum_ranks = dups = 0
    if pct:
        return ranks / count
    else:
        return ranks


def rank_2d_uint64(object in_arr, axis=0, ties_method='average',
                      ascending=True, na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, z, k, n, dups = 0, total_tie_count = 0


        ndarray[float64_t, ndim=2] ranks
        ndarray[uint64_t, ndim=2, cast=True] values

        ndarray[int64_t, ndim=2] argsorted

        uint64_t val

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0

    tiebreak = tiebreakers[ties_method]

    keep_na = na_option == 'keep'

    in_arr = np.asarray(in_arr)

    if axis == 0:
        values = in_arr.T.copy()
    else:
        values = in_arr.copy()


    n, k = (<object> values).shape
    ranks = np.empty((n, k), dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(axis=1, kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort(1)

    if not ascending:
        _as = _as[:, ::-1]

    values = _take_2d_uint64(values, _as)
    argsorted = _as.astype('i8')

    for i in range(n):
        dups = sum_ranks = 0

        total_tie_count = 0
        count = 0.0
        for j in range(k):
            sum_ranks += j + 1
            dups += 1

            val = values[i, j]


            count += 1.0

            if j == k - 1 or values[i, j + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = z + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = 2 * j - z - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = total_tie_count
                sum_ranks = dups = 0
        if pct:
            ranks[i, :] /= count
    if axis == 0:
        return ranks.T
    else:
        return ranks


@cython.wraparound(False)
@cython.boundscheck(False)


def rank_1d_int64(object in_arr, ties_method='average', ascending=True,
                      na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, n, dups = 0, total_tie_count = 0

        ndarray[int64_t] sorted_data, values

        ndarray[float64_t] ranks
        ndarray[int64_t] argsorted

        int64_t val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0
    tiebreak = tiebreakers[ties_method]

    values = np.asarray(in_arr)

    keep_na = na_option == 'keep'

    if ascending ^ (na_option == 'top'):
        nan_value = np.iinfo(np.int64).max
    else:
        nan_value = np.iinfo(np.int64).min

    mask = values == iNaT

    np.putmask(values, mask, nan_value)

    n = len(values)
    ranks = np.empty(n, dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort()

    if not ascending:
        _as = _as[::-1]

    sorted_data = values.take(_as)
    argsorted = _as.astype('i8')

    with nogil:
        for i in range(n):
            sum_ranks += i + 1
            dups += 1

            val = sorted_data[i]

            if (val == nan_value) and keep_na:
                ranks[argsorted[i]] = nan
                continue

            count += 1.0

            if i == n - 1 or sorted_data[i + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = i + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = j + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = 2 * i - j - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for j in range(i - dups + 1, i + 1):
                        ranks[argsorted[j]] = total_tie_count
                sum_ranks = dups = 0
    if pct:
        return ranks / count
    else:
        return ranks


def rank_2d_int64(object in_arr, axis=0, ties_method='average',
                      ascending=True, na_option='keep', pct=False):
    """
    Fast NaN-friendly version of scipy.stats.rankdata
    """

    cdef:
        Py_ssize_t i, j, z, k, n, dups = 0, total_tie_count = 0


        ndarray[float64_t, ndim=2] ranks
        ndarray[int64_t, ndim=2, cast=True] values

        ndarray[int64_t, ndim=2] argsorted

        int64_t val, nan_value

        float64_t sum_ranks = 0
        int tiebreak = 0
        bint keep_na = 0
        float count = 0.0

    tiebreak = tiebreakers[ties_method]

    keep_na = na_option == 'keep'

    in_arr = np.asarray(in_arr)

    if axis == 0:
        values = in_arr.T.copy()
    else:
        values = in_arr.copy()

    if ascending ^ (na_option == 'top'):
        nan_value = np.iinfo(np.int64).max
    else:
        nan_value = np.iinfo(np.int64).min

    mask = values == iNaT

    np.putmask(values, mask, nan_value)

    n, k = (<object> values).shape
    ranks = np.empty((n, k), dtype='f8')

    if tiebreak == TIEBREAK_FIRST:
        # need to use a stable sort here
        _as = values.argsort(axis=1, kind='mergesort')
        if not ascending:
            tiebreak = TIEBREAK_FIRST_DESCENDING
    else:
        _as = values.argsort(1)

    if not ascending:
        _as = _as[:, ::-1]

    values = _take_2d_int64(values, _as)
    argsorted = _as.astype('i8')

    for i in range(n):
        dups = sum_ranks = 0

        total_tie_count = 0
        count = 0.0
        for j in range(k):
            sum_ranks += j + 1
            dups += 1

            val = values[i, j]

            if (val == nan_value) and keep_na:
                ranks[i, argsorted[i, j]] = nan


                continue

            count += 1.0

            if j == k - 1 or values[i, j + 1] != val:
                if tiebreak == TIEBREAK_AVERAGE:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = sum_ranks / dups
                elif tiebreak == TIEBREAK_MIN:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j - dups + 2
                elif tiebreak == TIEBREAK_MAX:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = j + 1
                elif tiebreak == TIEBREAK_FIRST:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = z + 1
                elif tiebreak == TIEBREAK_FIRST_DESCENDING:
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = 2 * j - z - dups + 2
                elif tiebreak == TIEBREAK_DENSE:
                    total_tie_count += 1
                    for z in range(j - dups + 1, j + 1):
                        ranks[i, argsorted[i, z]] = total_tie_count
                sum_ranks = dups = 0
        if pct:
            ranks[i, :] /= count
    if axis == 0:
        return ranks.T
    else:
        return ranks
