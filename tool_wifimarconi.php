<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tts=$argv[1];
$tte=$argv[2];

for($tt=$tts;$tt<=$tte;$tt++){
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where tt=$tt and (ip>=14016 and ip<=14047)");
  oci_execute($query);
  $row=oci_fetch_row($query);
  printf("%d,%d\n",$tt,$row[0]);
}

oci_close($conn);

?>
