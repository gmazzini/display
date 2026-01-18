<?php

$ser=$_GET["ser"];
$bin="/run/display/$ser.bin";
@ini_set("zlib.output_compression","0");
@apache_setenv("no-gzip","1");
header("Content-Encoding: identity");
while (ob_get_level()) { ob_end_clean(); }
$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

?>
