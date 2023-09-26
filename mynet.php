<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

echo "idistat\n";
$stmt=oci_parse($conn,"delete from idistat");
oci_execute($stmt);

$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/gviz/tq?tq=select%20D%2CA%2CB&tqx=out:csv&gid=644800101"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  if(strlen($aa[0])<5)continue;
  $kk=substr($aa[0],1,5);
  if(!is_numeric($kk))continue;
  $vv=substr($aa[1],1,strlen($aa[1])-2);
  $qq=substr($aa[2],1,strlen($aa[2])-2);
  $stmt=oci_parse($conn,"insert into idistat values ($vv,$qq,'$kk','')");
  oci_execute($stmt);
}

oci_close($conn);

?>
