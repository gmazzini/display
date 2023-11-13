<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tts=$argv[1];
$tte=$argv[2];

$query=oci_parse($conn,"select distinct id from dhcpwifi where ip>=14016 and ip<=14047 and tt=$tts");
oci_execute($query);
for($i=0;;$i++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $id[$i]=$row[0];
}
oci_free_statement($query);
echo "user: $i\n";

for($j=0;$j<$i;$j++){
  $aux=$id[$j];
  $query=oci_parse($conn,"select istat from dhcpwifi where id='$aux' and tt>=$tts and tt<=$tte");
  for(;;){
    $row=oci_fetch_row($query);
    if($row==null)break;
    @$istat[$row[0]]++;
  }
}
oci_free_statement($query);

// arsort($aux);
foreach($istat as $kk => $vv){
  echo "$kk,$vv\n";
}

oci_close($conn);

?>
