<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tts=$argv[1];
$tte=$argv[2];

$query=oci_parse($conn,"select distinct id from dhcpwifi where (ip>=14016 and ip<=14047) and tt=$tts");
oci_execute($query);
for($i=0;;$i++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $id[$i]=$row[0];
}
oci_free_statement($query);
echo "user: $i\n";

for($tt=$tts+1;$tt<=$tte;$tt++){
  $ret[$tt]=0;
  for($j=0;$j<$i;$j++){
    $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where id='$id[$j]' and tt=$tt and (ip>=14016 and ip<=14047)");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if($row[0]>0)$ret[$tt]++;
    oci_free_statement($query);
  }
}

for($tt=$tts+1;$tt<=$tte;$tt++){
  if($ret[$tt]>=3)printf("%d,%d,%04.1f%%\n",$tt,$ret[$tt],(100.0*$ret[$tt])/$i);
}

oci_close($conn);

?>
