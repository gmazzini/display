<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select count(distinct istat) from dhcpwifi where id in (select distinct id from dhcpwifi)");
oci_execute($query);
for(;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  @$cc[$row[0]]++;
}
oci_free_statement($query);
print_r($cc);

oci_close($conn);

?>
