CREATE TABLE t1123132 (aap int);
INSERT INTO t1123132 VALUES (1),(2),(null);
SELECT MIN(aap),MAX(aap),SUM(aap) FROM t1123132;
