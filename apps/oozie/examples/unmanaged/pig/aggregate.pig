A = LOAD '$INPUT' USING PigStorage(',')  AS (word:CHARARRAY, count:INT);
STORE A INTO '$OUTPUT';

