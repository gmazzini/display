<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tt=(int)$argv[1];
$lab=array("11271"=>"Pres","11452"=>"AM38");

$query=oci_parse($conn,"select idstart,idend from idistat where eistat='00008'");
oci_execute($query);
for($cc=0;;$cc++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $idstart[$cc]=(int)$row[0];
  $idend[$cc]=(int)$row[1];
}
oci_free_statement($query);

for($c=0;$c<$cc;$c++){
  $query=oci_parse($conn,"select count(*) from dhcpwifi where ip>=$idstart[$c] and ip<=$idend[$c] and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $nn[$c]=(int)$row[0];
  oci_free_statement($query);
}

for($c=0;$c<$cc;$c++){
  printf("%d,%s,%d\n",$tt,$lab[$idstat[$c]],$nn[$c]);
}

oci_close($conn);
?>
