<?php

$mydes=$_GET["des"];
$fp=fopen("/home/www/www.chaos.cc/qq.des","w");
$mydes=str_replace("\\","\n",$mydes);
fprintf($fp,"%s",$mydes);
fclose($fp);
shell_exec("/home/www/www.chaos.cc/write2 /home/www/www.chaos.cc/qq.des 4 /home/www/www.chaos.cc/qq.ff; /home/w
ww/www.chaos.cc/convert3 /home/www/www.chaos.cc/qq.ff 1 /home/www/www.chaos.cc/qq.bin");
$len=filesize("/home/www/www.chaos.cc/qq.bin");
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile("/home/www/www.chaos.cc/qq.bin");


?>
