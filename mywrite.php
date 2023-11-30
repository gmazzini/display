<?php

$base="/home/www/www.chaos.cc/dev";
$mydes=$_GET["des"];
$fp=fopen("$base/qq.des","w");
$mydes=str_replace("\\","\n",$mydes);
fprintf($fp,"%s",$mydes);
fclose($fp);
shell_exec("$base/write2 $base/qq.des $base/qq.ff; $base/convert3 $base/qq.ff 1 $base/qq.bin");
$len=filesize("$base/qq.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("$base/qq.bin");

?>
