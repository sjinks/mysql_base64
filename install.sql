USE mysql;

DROP FUNCTION IF EXISTS gb_base64_encode;
DROP FUNCTION IF EXISTS gb_base64_decode;

CREATE FUNCTION gb_base64_encode RETURNS STRING SONAME 'udf_gbbase64.so';
CREATE FUNCTION gb_base64_decode RETURNS STRING SONAME 'udf_gbbase64.so';
