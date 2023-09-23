<?php

$conn=mysqli_connect("127.0.0.1","matrix","matrix123","matrix");

$res=mysqli_query($conn,"select distinct(sovra) from idistat where sovra<>''");
for($ss=0;;$ss++){
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  if($row==null)break;
  $sovra[$ss]=$row[0];
}
mysqli_free_result($res);

function fai1($conn,$table,$field,$url,$sovra,$ss){
  mysqli_query($conn,"delete from $table");
  $aux=json_decode(file_get_contents("$url"),true);
  foreach($aux["dati"] as $k => $v){
    $kk=substr($k,1,5);
    $vv=$v["$field"];
    mysqli_query($conn,"insert into $table values ('$kk','$vv')");
  }
  mysqli_query($conn,"insert into $table select '00008',sum($field) from $table");
  for($i=0;$i<$ss;$i++)mysqli_query($conn,"insert ignore into $table select idistat.sovra,sum($field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
}

fai1($conn,"attivifse","attivi","https://dati.fascicolo-sanitario.it/rest/attivi/comune",$sovra,$ss);
fai1($conn,"accessifse","accessi","https://dati.fascicolo-sanitario.it/rest/accessi/comune",$sovra,$ss);
fai1($conn,"scaricatifse","scaricati","https://dati.fascicolo-sanitario.it/rest/documenti/comune",$sovra,$ss);

fai1($conn,"attivazionilepidaid","attivazioni","https://dati.fascicolo-sanitario.it/rest/lepidaid/attivazioni/comune",$sovra,$ss);
fai1($conn,"accessilepidaid","accessi","https://dati.fascicolo-sanitario.it/rest/lepidaid/accessi/comune",$sovra,$ss);
fai1($conn,"sportellilepidaid","sportelli","https://dati.fascicolo-sanitario.it/rest/lepidaid/sportelli/comune",$sovra,$ss);


mysqli_close($conn);

?>
