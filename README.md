------------------------INTRODUCCIÓN------------------------

Programa para **Esp32**.
Botón de emergencia para negocios. Usa una aplicación externa llamada "ntfy" para mandar la notifiación de alerta. se hace una solicitud https usando POST para mandar una notificación a todos aquellos
suscritos a un grupo en la app. 
Funciona obligatoriamente con WiFi, con capacidad de almacenar dos redes: Una principal (Tu internet) y otra de respaldo (Hotspot de tu celular)
Cuenta con un respaldo de energía en caso de que no haya luz.

------------------------CONFIGURACIÓN DE GRUPO------------------------

Para crear un grupo en la app no necesitas inicio de sesión, simplemente presiona el botón con el ícono '+', asignas nombre a tu grupo
y presionas la opción "suscribir".

***RECOMENDACIÓN***
Los grupos no son protegidos por contraseñas, así que el nombre de tu grupo es tu contraseña, entre más complicado sea, mejor.
Por ello se recomienda que tu nombre del grupo contenga:
-Mayúsculas
-Nombre mayor a 15 caracteres
-Números
-Caracteres expeciales: ()-+@/.

Para meter a más personas al grupo, tienen que seguir los mismos pasos que tu.

------------------------CONFIGURACIÓN DEL BOTÓN------------------------

Cuenta con dos leds: Uno verde y uno rojo (de preferencia estos colores).
1.-Dejar presionado el botón en los primeros 3 segundos que se conecte a corriente. Ambos leds prenderán y apagarán, para dejar el led rojo
encendido, eso significa que el botón entro en modo configuración.
que el botón está en modo configuración.
2.-Entra a tus ajustes de WiFi y verás una red llamda ALAMAR CONFIG. Conectate a esa red y de forma automática se abrira el portal de
configuración de red. Si no se abre automáticamente, entra a tu navegador y escribe la siguiente IP: 192.168.4.1
3.-En el portal seleccionarás la opción: "Configurar WiFi" (o algo similar).
4.-Después de seleccionar, verás una lista de redes WiFi disponibles. Busca tu red WiFi y seleccionala, introduce la contraseña de tu red.
5.-Una vez seleccionada la red WiFi, llenarás un pequeño formulario que te pedirá:
  -Nombre del negocio.
  -Dirección.
Estos serán los datos que les llegarán en las notificaciones a las personas suscritas a tu grupo.
6.-De forma opcional puedes agregar una red de respaldo (preferiblemente la del hotspot de tu celular) esto en caso de que no haya 
internet o se te haya ido la luz.
7.-Presiona la opción "Guardar" y espera a que el led verde encienda y el rojo apague.

¡¡¡FELICIDADES, TU BOTÓN DE EMERGENCIA ESTÁ CONFIGURADO Y LISTO PARA FUNCIONAR!!!







