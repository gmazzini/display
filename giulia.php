<?php

$base="/run/display/giulia";
$des=$_GET["des"];
$fp=fopen("$base.des","w");
$des=str_replace("\\","\n",$des);
fprintf($fp,"%s",$des);
fclose($fp);
shell_exec("/home/www/display/write4 $base.des $base.ff $base.bin");
$len=filesize("$base.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("$base.bin");

?>
