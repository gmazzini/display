<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tts=$argv[1];
$tte=$argv[2];
$thr=$argv[3];

$query=oci_parse($conn,"select distinct id from dhcpwifi where (ip>=14016 and ip<=14047) and tt=$tts");
oci_execute($query);
for(;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $id[$row[0]]=1;
}
oci_free_statement($query);
$tot1=count($id);
echo "users on $tts: $tot1\n";

foreach($id as $k => $v){
  $query=oci_parse($conn,"select count(distinct tt) from dhcpwifi where id='$k' and (tt>=$tts and tt<=$tte) and (ip>=14016 and ip<=14047)");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if($row[0]>=$thr)unset($id[$k]);
  oci_free_statement($query);
}
$tot2=count($id);
echo "users below $thr on $tts: $tot2\n";

foreach($id as $k => $v){
  $query=oci_parse($conn,"select istat from dhcpwifi where id='$k' and (tt>=$tts and tt<=$tte) and not (ip>=14016 and ip<=14047)");
  oci_execute($query);
  for(;;){
    $row=oci_fetch_row($query);
    if($row==null)break;
    @$istat[$row[0]]++;
  }
  oci_free_statement($query);
}

arsort($istat);
foreach($istat as $kk => $vv){
  if($vv>=3)echo "$vv,$kk\n";
}

oci_close($conn);

?>
