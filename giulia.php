<?php

$base="/run/display/giulia";
$fp=fopen("$base.des","w");
$des=str_replace("\\","\n",$_GET["des"]);
fprintf($fp,"%s",$des);
fclose($fp);
shell_exec("/home/www/display/write3 $base.des $base.ff $base.bin");
$len=filesize("$base.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("$base.bin");

?>
