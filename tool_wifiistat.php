<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$te=(int)(time()/86400);
$ts=$te-$argv[1];
$istat=$argv[2];

$query=oci_parse($conn,"select idstart,idend from idistat where eistat='$istat'");
oci_execute($query);
for($cc=0;;$cc++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $idstart[$cc]=(int)$row[0];
  $idend[$cc]=(int)$row[1];
}
oci_free_statement($query);

for($tt=$ts;$tt<$te;$tt++){
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where istat='$istat' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $nn=(int)$row[0];
  oci_free_statement($query);

  $enn=0;
  for($c=0;$c<$cc;$c++){
    $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where tt=$tt and ip>=$idstart[$c] and ip<=$idend[$c]");
    oci_execute($query);
    $row=oci_fetch_row($query);
    $enn+=(int)$row[0];
    oci_free_statement($query);
  }

  echo "$tt,$nn,$enn\n";
}

oci_close($conn);
?>
