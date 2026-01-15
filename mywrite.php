<?php

$base="/home/www/display/";
$mydes=$_GET["des"];
$fp=fopen("$base/dev/qq.des","w");
$mydes=str_replace("\\","\n",$mydes);
fprintf($fp,"%s",$mydes);
fclose($fp);
shell_exec("$base/write2 $base/dev/qq.des $base/dev/qq.ff; $base/convert3 $base/dev/qq.ff 1 $base/dev/qq.bin");
$len=filesize("$base/dev/qq.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("$base/dev/qq.bin");

?>
