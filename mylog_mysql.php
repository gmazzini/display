<?php

$start=30000000;

$conn=mysqli_connect("127.0.0.1","matrix","matrix123","matrix");

for($i=42;$i<=63;$i++){
  for($j=0;$j<=255;$j++){
    $id=$i*256+$j;
    $res=mysqli_query($conn,"select istat from idistat where '$id'>=idstart and '$id'<=idend");
    $row=mysqli_fetch_array($res,MYSQLI_NUM);
    @$myistat[$id]=$row[0];
    mysqli_free_result($res);
  }
}

$ii=0;
$fp=fopen("ee","r");
for($i=1;;$i++){
  $aux=fgets($fp);
  if(feof($fp))break;
  if($i<$start)continue;
  $aa=explode(",",$aux);
  $res=mysqli_query($conn,"select count(*) from logwifi where id='$aa[0]' and ip=$aa[1]");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  $myexist=$row[0];
  mysqli_free_result($res);
  if(!$myexist){
    $istat=$myistat[$aa[1]];
    mysqli_query($conn,"insert into logwifi values ('$aa[0]',$aa[1],'$aa[2]','$istat')");
    $ii++;
    echo "$i,$ii\n";
  }
}
fclose($fp);

mysqli_close($conn);

?>
