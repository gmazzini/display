<?php

$ip=$_GET["ip"];
@$ser=$_GET["ser"];
if($ser=="")$ser="0000";
$des="(/home/www/display/files/$ip.des";
$ff="/home/www/display/files/$ip.ff";
$bin="/home/www/display/files/$ip.bin";

$fp=fopen($des,"w");
fprintf($fp,"1 -2 0 FF0000FF 00000000 3 %sZ\n",date("d.m.y H:i"));
fprintf($fp,"1 -2 6 FFFFFFFF 00000000 3 %s %s\n",$ip,$ser);
shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff 5 $bin");

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

?>
