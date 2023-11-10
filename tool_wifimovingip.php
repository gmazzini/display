<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select count(distinct ip),id from dhcpwifi group by id");
oci_execute($query);
for(;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  @$cc[$row[0]]++;
}
oci_free_statement($query);
for($i=1;$i<50;$i++){
  if(isset($cc[$i]))$oo=(int)$cc[$i];
  else $oo=0;
  echo "$i,$oo\n";
}

oci_close($conn);
?>
