<?php

include "/home/admgm02/data.php";
$conn=oci_connect($p1,$p2,$p3);

echo "<pre>";
if($_GET["password"]!=$p4){
  echo "Wrong password\n";
  exit(0);
}

echo "Updating sel\n";
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1JOdvGEDQl6L5Zr3b3ir5OO8tjJhWOfRTML5Rl4jskP4/gviz/tq?tq=select%20A%2CB&tqx=out:csv&gid=0"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  $ser=substr($aa[0],1,strlen($aa[0])-2);
  $seq=substr($aa[1],1,strlen($aa[1])-2);
  if($seq=="")$seq="0000";
  $query=oci_parse($conn,"select count(*) from sel where ser='$ser'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  if($zz)$query=oci_parse($conn,"update sel set seq='$seq' where ser='$ser'");
  else $query=oci_parse($conn,"insert into sel (ser,seq) values ('$ser','$seq')");
  oci_execute($query);
  oci_free_statement($query);
}

echo "Updating seq\n";
$query=oci_parse($conn,"delete from seq");
oci_execute($query);
oci_free_statement($query);
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1JOdvGEDQl6L5Zr3b3ir5OO8tjJhWOfRTML5Rl4jskP4/gviz/tq?tq=select%20A%2CB%2CC%2CD&tqx=out:csv&gid=324624767"));
for($i=1;;$i++){
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
  if($zz)$query=oci_parse($conn,"update seq set screen='$screen',time=$time where seq='$seq' and id=$id");
  else $query=oci_parse($conn,"insert into seq (seq,id,screen,time) values ('$seq',$id,'$screen',$time)");
  oci_execute($query);
  oci_free_statement($query);
}

oci_close($conn);

?>
