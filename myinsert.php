<?php

$start=1;

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

for($i=42;$i<=63;$i++){
  for($j=0;$j<=255;$j++){
    $id=$i*256+$j;
    $query=oci_parse($conn,"select istat from idistat where $id>=idstart and $id<=idend");
    oci_execute($query);
    $row=oci_fetch_row($query);
    @$myistat[$id]=$row[0];
    oci_free_statement($query);
  }
}

$ii=0;
$fp=fopen("ee","r");
for($i=1;;$i++){
  $aux=fgets($fp);
  if(feof($fp))break;
  if($i<$start)continue;
  $aa=explode(",",$aux);
  
  $tt=strtotime($aa[2]);
  $query=oci_parse($conn,"select count(*) from logwifi where id='$aa[0]' and ip=$aa[1] and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$myexist=$row[0];
  oci_free_statement($query);

  if(!$myexist){
    $istat=$myistat[$aa[1]];
    $stmt=oci_parse($conn,"insert into logwifi values ('$aa[0]',$aa[1],$tt,'$istat')");
    oci_execute($stmt);
    $ii++;
    echo "$i,$ii\n";
  }
}
fclose($fp);

oci_close($conn);

?>
