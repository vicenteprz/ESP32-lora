
<h1 align="center">
  Red LoRa Con ESP32 para captura de datos climáticos para 
  actividades economicas rurales en los alrededores de Temuco
</h1>

------

Este proyecto corresponde al trabajo de titulo realizado por Vicente Pérez para la carrera de Ingeniería Civil en Informática de la Universidad Catolica de Temuco con el Profesor Guía Luis Alberto Caro.

------

Se hicieron 3 Demos para ESP32 que trabajan en conjunto para crear una red usando LoRa donde los nodos Transmisores mandan datos obtenidos por sensores a un nodo Recividor el cual se encarga de mandar los datos recibidos por medio del protocolo MQTT a un servidor que está corriendo Mosquitto, Node-Red, Grafana y MariaDB . Los datos son así guardados y graficados por medio de los Dashboards. 

![Diagrama de interacción de sistemas](/images/IntegracionSistemas.png)


1. [Demo Recividor T-BEAM](DEMO_CSV_receiver_TTGO_TBEAM/DEMO_CSV_receiver_TTGO_TBEAM.ino)

2. [Demo Transmisor WifiLoraV2](DEMO_CSV_Sender_WifiLoRaV2/DEMO_CSV_Sender_WifiLoRaV2.ino)

3. [Demo Transmisor Wireless Stick 2.1](DEMO_CSV_Sender_WirelessStick21/DEMO_CSV_Sender_WirelessStick21.ino)

Los códigos para replicar el ambiente de Node-Red y Grafana se encuentran disponibles en carpeta raíz.


Se puede ver : [Dashboard Público con Grafana](http://localhost:3000/public-dashboards/94c0bb087d04442fb0c60637d3966544)

Adicionalmente también están guardados en carpetas códigos antiguos  correspondientes a las etapas 2, 3 y 4. 

Una Demo que puede resultar útil es la que se utilizó para hacer funcionar el PMS7003 con un Heltec ESP32 Wireless Stick 2.1

[Demo para utilizar PMS7003](Demos Ultima etapa/Demo_PMS7003/Demo_PMS7003.ino)

 
