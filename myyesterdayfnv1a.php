<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$tt=(int)(time()/86400)-1;
$query=oci_parse($conn,"select distinct rawtohex(fnv1a) from dhcpwifi where tt=$tt and fnv1a not in (select fnv1a from dhcpwifi where tt<$tt and fnv1a is not null) order by fnv1a");
oci_execute($query);
$ii=0;
for($i=0;;$i++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $fnv1a=$row[0];
  $query1=oci_parse($conn,"select count(distinct id) from dhcpwifi where fnv1a=hextoraw('$fnv1a')");
  oci_execute($query1);
  $row1=oci_fetch_row($query1);
  $nn=$row1[0];
  oci_free_statement($query1);
  if($nn==1){
    $query1=oci_parse($conn,"select id from dhcpwifi where fnv1a=hextoraw('$fnv1a')");
    oci_execute($query1);
    $row1=oci_fetch_row($query1);
    $id=$row1[0];
    oci_free_statement($query1);
    $query1=oci_parse($conn,"update dhcpwifi set fnv1a=hextoraw('$fnv1a') where id='$id' and fnv1a is not null");
    oci_execute($query1);
    $mm=oci_num_rows($query1);
    oci_free_statement($query1);
    echo "$i $ii $row[0] $id $mm\n";
    $ii++;
  }
}
oci_free_statement($query);

oci_close($conn);

?>
