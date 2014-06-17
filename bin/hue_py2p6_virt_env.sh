#!/bin/bash 
bin=`dirname "$0"`
bin=`cd "$bin"; pwd`
export HUE_HOME=${bin}/..
cd $HUE_HOME/build/env/lib/python*/
rm _weakrefset.pyo
rm _weakrefset.py
rm warnings.pyo
rm warnings.py
rm UserDict.pyo
rm UserDict.py
rm types.pyo
rm types.py
rm stat.pyo
rm stat.py
rm sre.pyo
rm sre.py
rm sre_parse.pyo
rm sre_parse.py
rm sre_constants.pyo
rm sre_constants.py
rm sre_compile.pyo
rm sre_compile.py
rm re.pyo
rm re.py
rm posixpath.pyo
rm posixpath.py
rm os.pyo
rm os.py
rm ntpath.pyo
rm ntpath.py
rm locale.pyo
rm locale.py
rm linecache.pyo
rm linecache.py
rm lib-dynload
rm genericpath.pyo
rm genericpath.py
rm fnmatch.pyo
rm fnmatch.py
rm encodings
rm copy_reg.pyo
rm copy_reg.py
rm config
rm codecs.pyo
rm codecs.py
rm abc.pyo
rm abc.py
rm _abcoll.pyo
rm _abcoll.py

rm _abcoll.pyc
rm abc.pyc
rm codecs.pyc
rm copy_reg.pyc
rm fnmatch.pyc
rm genericpath.pyc
rm linecache.pyc
rm locale.pyc
rm ntpath.pyc
rm os.pyc
rm posixpath.pyc
rm re.pyc
rm sre_compile.pyc
rm sre_constants.pyc
rm sre_parse.pyc
rm sre.pyc
rm stat.pyc
rm types.pyc
rm UserDict.pyc
rm warnings.pyc
rm _weakrefset.pyc

rm -rf  config 
rm -rf  encodings 
rm -rf  lib-dynload 

cp  /usr/lib/python2.6/_weakrefset.py _weakrefset.py
cp  /usr/lib/python2.6/warnings.py warnings.py
cp  /usr/lib/python2.6/UserDict.py UserDict.py
cp  /usr/lib/python2.6/types.py types.py
cp  /usr/lib/python2.6/stat.py stat.py
cp  /usr/lib/python2.6/sre.py sre.py
cp  /usr/lib/python2.6/sre_parse.py sre_parse.py
cp  /usr/lib/python2.6/sre_constants.py sre_constants.py
cp  /usr/lib/python2.6/sre_compile.py sre_compile.py
cp  /usr/lib/python2.6/re.py re.py
cp  /usr/lib/python2.6/posixpath.py posixpath.py
cp  /usr/lib/python2.6/os.py os.py
cp  /usr/lib/python2.6/ntpath.py ntpath.py
cp  /usr/lib/python2.6/locale.py locale.py
cp  /usr/lib/python2.6/linecache.py linecache.py
cp  /usr/lib/python2.6/genericpath.py genericpath.py
cp  /usr/lib/python2.6/fnmatch.py fnmatch.py
cp  /usr/lib/python2.6/copy_reg.py copy_reg.py
cp  /usr/lib/python2.6/codecs.py codecs.py
cp  /usr/lib/python2.6/abc.py abc.py
cp  /usr/lib/python2.6/_abcoll.py _abcoll.py

cp  /usr/lib/python2.6/_weakrefset.pyc _weakrefset.pyc
cp  /usr/lib/python2.6/warnings.pyc warnings.pyc
cp  /usr/lib/python2.6/UserDict.pyc UserDict.pyc
cp  /usr/lib/python2.6/types.pyc types.pyc
cp  /usr/lib/python2.6/stat.pyc stat.pyc
cp  /usr/lib/python2.6/sre.pyc sre.pyc
cp  /usr/lib/python2.6/sre_parse.pyc sre_parse.pyc
cp  /usr/lib/python2.6/sre_constants.pyc sre_constants.pyc
cp  /usr/lib/python2.6/sre_compile.pyc sre_compile.pyc
cp  /usr/lib/python2.6/re.pyc re.pyc
cp  /usr/lib/python2.6/posixpath.pyc posixpath.pyc
cp  /usr/lib/python2.6/os.pyc os.pyc
cp  /usr/lib/python2.6/ntpath.pyc ntpath.pyc
cp  /usr/lib/python2.6/locale.pyc locale.pyc
cp  /usr/lib/python2.6/linecache.pyc linecache.pyc
cp  /usr/lib/python2.6/genericpath.pyc genericpath.pyc
cp  /usr/lib/python2.6/fnmatch.pyc fnmatch.pyc
cp  /usr/lib/python2.6/copy_reg.pyc copy_reg.pyc
cp  /usr/lib/python2.6/codecs.pyc codecs.pyc
cp  /usr/lib/python2.6/abc.pyc abc.pyc
cp  /usr/lib/python2.6/_abcoll.pyc _abcoll.pyc

cp -R /usr/lib/python2.6/config .
cp -R /usr/lib/python2.6/encodings .
cp -R /usr/lib/python2.6/lib-dynload .
