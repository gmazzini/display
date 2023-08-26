<?php

$ip=$_GET["ip"];
$db="tmp/db/$ip";
$des="tmp/des/$ip.des";
$ff="tmp/ff/$ip.ff";
$bin="tmp/bin/$ip.bin";

$fp=fopen($db,"r");
$vv=intval(fgets($fp));
fclose($fp);
$vv++;
if($vv>83*2)$vv=1;
$fp=fopen($db,"w");
fprintf($fp,"%d\n",$vv);
fclose($fp);

if($vv%2==0){
  $vf=$vv/2;
  $name=sprintf("tmp/img/%03d.ff",$vf);
  shell_exec("tmp/convert3 $name 7 $bin");
}
else {
  $fp=fopen($des,"w");
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 400040 02 Lepida\n");
  fprintf($fp,"-2 10 FF0000 02 %s\n",date("d.m.y H:i"));
  fprintf($fp,"-2 18 FFFFFF 02 %s\n",$ip);
  fprintf($fp,"-2 28 FF00FF 02 Utenti WiFi\n");
  fprintf($fp,"-2 36 00FF00 02 %s\n","6.123.456");
  fprintf($fp,"-2 46 FFFF00 02 Punti WiFi\n");
  fprintf($fp,"-2 54 0000FF 02 %s\n","11.234");
  fclose($fp);
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff 11 $bin");
}

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);


?>
