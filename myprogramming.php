<?php

include "/home/admgm02/data.php";
$conn=oci_connect($p1,$p2,$p3);

function mycheck($conn,$table,$istat){
  $query=oci_parse($conn,"select count(*) from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  return $zz; 
}

echo "sel\n";
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1JOdvGEDQl6L5Zr3b3ir5OO8tjJhWOfRTML5Rl4jskP4/gviz/tq?tq=select%20A%2CB&tqx=out:csv&gid=0"));
for($;;$){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  $sel=substr($aa[0],1,strlen($aa[0])-2);
  $seq=substr($aa[1],1,strlen($aa[1])-2);
  if($seq=="")$seq="0000";
  $query=oci_parse($conn,"select count(*) from sel where sel='$sel'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  if($zz)$query=oci_parse($conn,"update sel set seq='$seq' where sel='$sel'");
  else $query=oci_parse($conn,"insert into sel (sel,seq) values ('$sel','$seq')");
  oci_execute($query);
  oci_free_statement($query);
}

echo "seq\n";
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1JOdvGEDQl6L5Zr3b3ir5OO8tjJhWOfRTML5Rl4jskP4/gviz/tq?tq=select%20A%2CB%2CC%2CD&tqx=out:csv&gid=324624767"));
for($;;$){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  $seq=substr($aa[0],1,strlen($aa[0])-2);
  $id=(int)substr($aa[1],1,strlen($aa[1])-2);
  $screen=substr($aa[2],1,strlen($aa[2])-2);
  $time=(int)substr($aa[3],1,strlen($aa[3])-2);
  $query=oci_parse($conn,"select count(*) from seq where seq='$seq' and id=$id");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  if($zz)$query=oci_parse($conn,"update seq set screen='$screem',time=$time where seq='$seq' and id=$id");
  else $query=oci_parse($conn,"insert into seq (seq,id,screen,time) values ('$seq',$id,'$screen',$time)");
  oci_execute($query);
  oci_free_statement($query);
}

oci_close($conn);

?>
