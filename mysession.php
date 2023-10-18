<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

echo "<pre><table>";
$query=oci_parse($conn,"select id,ser,iter,c1,c2 from mysession order by ser");
oci_execute($query);
for(;;){
  $row=oci_fetch_row($query);
  if($row==null)break;

  $aux=explode(".",$row[0]);
  if($aux[0]=="10"){
    $id=$aux[1]*256+$aux[2];
    $query1=oci_parse($conn,"select istat from idistat where '$id'>=idstart and '$id'<=idend");
    oci_execute($query1);
    $row1=oci_fetch_row($query1);
    $istat=$row1[0];
    oci_free_statement($query1);
  }
  else if($aux[0]=="WEB")$istat=$aux[3];
  else $istat="37006";
  $query1=oci_parse($conn,"select sovra from idistat where istat='$istat'");
  oci_execute($query1);
  $row1=oci_fetch_row($query1);
  @$sovra=$row1[0];
  oci_free_statement($query1);

  echo "<tr><td>$row[0]</td><td>$row[1]</td><td>$row[2]</td><td>$row[3]</td><td>$row[4]</td><td>$istat</td><td>$sovra</td></tr>";
}
oci_free_statement($query);
echo "</table>";

oci_close($conn);

?>
