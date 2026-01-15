<?php

$base="/home/www/display/";
$myf=$base."files/";
$mydes=$_GET["des"];
$fp=fopen("$myf/qq.des","w");
$mydes=str_replace("\\","\n",$mydes);
fprintf($fp,"%s",$mydes);
fclose($fp);
shell_exec("$base/write2 $myf/qq.des $myf/qq.ff; $base/convert3 $myf/qq.ff 10 $myf/qq.bin");
$len=filesize("$myf/qq.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("$myf/qq.bin");

?>
