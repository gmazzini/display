<?php

$ip=$_GET["ip"];
$ser=$_GET["ser"];
$bin="/home/www/display/files/$ip.bin";
$files=glob('/home/www/display/images/[0-9][0-9][0-9][0-9].ff');
shuffle($files);
$ff=$files[0];
shell_exec("/home/www/display/convert3 $ff 40 $bin");

@ini_set('zlib.output_compression', '0');
@apache_setenv('no-gzip', '1');
header('Content-Encoding: identity');
while (ob_get_level()) { ob_end_clean(); }
$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);


function next_display($ser){
  @mkdir("/run/display",0777,true);
  $f="/run/display/$ser";
  $n=(int)@file_get_contents($f)+1;
  file_put_contents($f,(string)$n,LOCK_EX);
  return $n;
}

?>
