<?php

$ip=$_GET["ip"];
$ser=$_GET["ser"];
@mkdir("/run/display",0777,true);
$bin="/run/display/$ser.bin";
$des="/run/display/$ser.des";
$ff="/run/display/$ser.ff";
$n=(int)@file_get_contents("/run/display/$ser.step")+1;
file_put_contents("/run/display/$ser.step",(string)$n."\n",LOCK_EX);

$files=glob("/home/www/display/images/[0-9][0-9][0-9][0-9].ff");
shuffle($files);
// $ff=$files[0];

file_put_contents($des,"#\n1 0 0 FF0000FF 00000000 4 $n\n");

shell_exec("/home/www/display/write2 $des $ff; /home/www/display/convert3 $ff 40 $bin");

@ini_set("zlib.output_compression","0");
@apache_setenv("no-gzip","1");
header("Content-Encoding: identity");
while (ob_get_level()) { ob_end_clean(); }
$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

?>
