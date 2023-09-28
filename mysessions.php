<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select * from mysession");
oci_execute($query);
for(;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  echo "$row[0] $row[1] $row[2] $row[3] $row[4]\n";
}
oci_free_statement($query);

oci_close($conn);

?>
