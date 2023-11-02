<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$te=(int)(time()/86400);
$ts=$te-$argv[1];
$istat=$argv[2];

for($tt=$ts;$tt<$te;$tt++){
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where istat='$kk' and tt=");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $nn=(int)$row[0];
  oci_free_statement($query);
  echo "$tt,$nn\n";
}

oci_close($conn);
?>
