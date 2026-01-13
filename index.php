<?php

$ip=$_GET["ip"];
@$ser=$_GET["ser"];
if($ser=="")$ser="0000";
$des="/home/www/display/files/$ip.des";
$ff="/home/www/display/files/$ip.ff";
$bin="/home/www/display/files/$ip.bin";

$fp=fopen($des,"w");
fprintf($fp,"1 -2 0 FF0000FF 00000000 3 %sZ\n",date("d.m.y H:i:s"));
fprintf($fp,"1 -2 6 FFFFFFFF 00000000 3 %s %s\n",$ip,$ser);
fprintf($fp,"1 -2 12 00FF00FF 00000000 3 Ciao a tutti\n");
shell_exec("/home/www/display/write2 $des $ff; /home/www/display/convert3 $ff 8 $bin");

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

?>
