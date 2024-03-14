<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$ips=$argv[1];
$ipe=$argv[2];

$query0=oci_parse($conn,"select distinct tt from dhcpwifi order by tt");
oci_execute($query0);
for(;;){
  $row=oci_fetch_row($query0);
  if($row==null)break;
  $tt=$row[0];
  
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where tt=$tt and ip>=$ips and ip<=$ipe");
  oci_execute($query);
  $row=oci_fetch_row($query);
  echo "$tt,$row[0]\n";
  oci_free_statement($query);
}
oci_free_statement($query0);

oci_close($conn);

?>
