<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select count(distinct tt) from dhcpwifi where id in (select distinct id from dhcpwifi) group by id");
oci_execute($query);
for($i=0;;$i++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $ff=$row[0];
  @$aux[$ff]++;
}
oci_free_statement($query);

arsort($aux);
foreach($aux as $kk => $vv){
  echo "$kk,$vv\n";
}

oci_close($conn);

?>
