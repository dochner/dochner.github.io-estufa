<?php
$page =
$_SERVER['PHP_SELF'];
$sec = '3';
$T=file_get_contents("controle_temperatura.txt");
$I=file_get_contents("controle_iluminacao.txt");
?>
<html>

    <head>
        <meta http equiv="refresh" content="<?php echo $sec?>;URL='<?php echo $page?>'">
        <style type="text/css">
            @importurl("styles.css");
            /* @importurl("onOff.css"); */
        </style>
        <?php
    if(isset($_POST["botao_foto"])){
    echo shell_exec('fswebcam r 960x720 d /dev/video1 foto1.jpg');
    echo "Tirando a foto...";
}
    if(isset($_POST["botao_temp_inc"])){
        if($T>= 30){
            echo "Valor maximo= 30 oC";
        }else{
            echo "Aumentando temperatura...";
            $T++;
            echo $T;
            file_put_contents("controle_temperatura.txt", $T);
        }
    if(isset($_POST["botao_temp_dec"])){
        if($T<=20){
            echo "Valor minimo= 20 oC";
        } else{
            echo "Diminuindo temperatura...";
            $T--;
            echo $T;
            file_put_contents("controle_temperatura.txt", $T);
        }
    }   

    if(isset($_POST["botao_irrigar"])){
    echo "Irrigação ativada....";
    file_put_contents("controle_irrigacao.txt", "l");
}
if(!empty($_POST["onoffswitch"])){
    echo "On/Off apertado...";
    // echo shell_exec('sh irrigar.sh');

}

if(isset($_POST["botao_luz"])){
    if($I == 0 ){
        echo "Acendendo a luz...";
        $I = 1;
        file_put_contents("controle_iluminacao.txt", $I);

    }else{
        echo "Apagando a luz...";
        $I = 0;
        file_put_contents("controle_iluminacao.txt", $I);
    }
}

?>
    </head>

    <body id="corpo">
        <div align="center" id="bl">
            <img src="banner1.jpg" align="middle"></img>
        </div>
        <h5></h5>
        <?php
    $str = file_get_contents("arquivo.txt");
    list($temperatura, $umidade, $umidadeSolo, $nivelDaAgua) = explode("|", $str);
    list($temp, $ntemp)=explode(":", $temperatura);
    list($umid, $numid)=explode(":", $umidade);
    list($umidS, $numidS)=explode(":", $umidadeSolo);
    list($nivel, $nnivel)=explode(":", $nivelDaAgua);
        if($I == 0){
            $nilum="OFF";
        }else{
            $nilum="ON";
            $ilum="LUZ";
        }
        /* if($nnivel<100){
            $enivel="CRITICO";
        }else{
            $enivel="OK"
        } */
        echo '<tablewidth="30%" align="center" id="tabela" style="font-family:arial;color:black;font-size:x-large;">';
        echo '<thead>>tr>';
        echo '</tr></thead>';
        echo '<tbody>';
        echo '<tr>';
        echo '<td><b>' . "TEMPERATURA" . '</b></td>';
        echo '<td><b>' . $ntemp . " C" . '</b>/td>';
        echo '</tr>';
        echo '<tr>';
        echo '<td><b>' . "UMIDADE" . '</b></td>';
        echo '<td><b>' . $numid . " %" . '</b>/td>';
        echo '</tr>';
        echo '<tr>';
        echo '<td><b>' . "ILUMINACAO" . '</b></td>';
        echo '<td><b>' . $nilum  . '</b>/td>';
        echo '</tr>';
        echo '<tr>';
        echo '<td><b>' . "NIVEL DA AGUA" . '</b></td>';
        echo '<td><b>' . $nnivel . '</b>/td>';
        echo '</tr>';

        echo'</tbody></table>'
        ?>

            <div id=painel>
                <h1>Painel de controle</h1>
                <div class="caixa_painel"></div>
                <div align="center" id="foto">
                    <img src="foto1.jpg" align="middle" width="300" height="200">
                </div>
                <form align="center" id="Botao" name="botao_foto" method="POST" action="index.php">
                    <input type="image" src="img_camera.jpg" name="botao_foto" id="botao_maquina" value="Tirar Foto!" />
                </form>
                <form align="center" id="Tinc" name="botao_temp_inc" method="POST" action="index.php">
                    <input type="image" src="img_tmp_inc.jpg" name="botao_temp_inc" id="Tinc" value="+" />
                </form>

                <div id="Techo">
                    <h2></h2>
                    <?php echo "$T"; ?>
                </div>

                <form align="center" id="Tdec" name="botao_temp_dec" method="POST" action="index.php">
                    <input type="image" src="img_temp_dec.png" name="botao_temp_dec" id="Tdec" value="-" />
                </form>

                <form align="center" id="irrigar" name="botao_irrigar" method="POST" action="index.php">
                    <input type="image" src="img_regador.jpg" name="botao_irrigar" id="irrigar" value="Irrigar" />
                </form>

                <form align="center" id="luz" name="botao_luz" method="POST" action="index.php">
                    <input type="image" src="img_luz.jpg" name="botao_luz" id="luz" value="Irrigar" />
                </form>


            </div>
            <div align="center" id="b2">
                <img src="img_banner2.jpg" align="middle" alt="">
            </div>

        <?php } ?>

    </body>

</html>