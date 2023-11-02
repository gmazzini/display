<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$te=(int)(time()/86400);
$ts=$te-30;

$query=oci_parse($conn,"select idstart,idend,eistat from idistat order by idstart");
oci_execute($query);
for($cc=0;;$cc++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $idstart[$cc]=(int)$row[0];
  $idend[$cc]=(int)$row[1];
  $eistat[$cc]=$row[2];
  $query1=oci_parse($conn,"select ente from istatente where istat='$eistat[$cc]'");
  oci_execute($query1);
  $row1=oci_fetch_row($query1);
  $ente[$cc]=$row1[0];
  oci_free_statement($query1);
}
oci_free_statement($query);

for($c=0;$c<$cc;$c++){
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where tt>=$ts and tt<=$te and ip>=$idstart[$c] and ip<=$idend[$c]");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $enn=(int)$row[0];
  oci_free_statement($query);
  if($enn==0){
    $ip=sprintf("10.%d.%d.0/%d",(int)$idstart[$c]/256,$idstart[$c]%256,24-log($idend[$c]-$idstart[$c]+1)/log(2));
    echo "$ip,$idstart[$c],$idend[$c],$eistat[$c],$ente[$c]\n";
  }
}

oci_close($conn);
?>
