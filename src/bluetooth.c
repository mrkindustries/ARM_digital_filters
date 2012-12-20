#include "bluetooth.h"
//#include "misc.h"
#include "frontend.h"
#include "timersystem.h"
//#include "utils.h"
#include "semihosting.h"


//static unsigned char rx_buffer[256];
//static unsigned int rx_put_idx = 0;
//static unsigned int rx_get_idx = 0;
static bt_state btState;
//static bool makingEcg;
#define  atLineLen  16
static char atLine[atLineLen]; // Linea finalizada
static uint8_t atLinePos;

void configure_UART(uint32_t baud)
{
	USART_InitTypeDef configUSART;
    configUSART.USART_BaudRate = baud;
    configUSART.USART_WordLength = USART_WordLength_8b;
    configUSART.USART_StopBits = USART_StopBits_1;
    configUSART.USART_Parity = USART_Parity_No;
    configUSART.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    configUSART.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &configUSART);
    USART_Cmd(USART2, ENABLE);

}

void init_bluetooth()
{
    // Activar clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitTypeDef configTx;
    configTx.GPIO_Pin = GPIO_Pin_2; // PA2
    configTx.GPIO_Mode = GPIO_Mode_AF_PP;
    configTx.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &configTx);

    GPIO_InitTypeDef configRx;
    configRx.GPIO_Pin = GPIO_Pin_3; // PA3
    configRx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    configRx.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &configRx);

    configRx.GPIO_Pin = GPIO_Pin_1; // PA1, CTS
    configRx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    configRx.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &configRx);

    configRx.GPIO_Pin = GPIO_Pin_0; // PA0, RTS
    configRx.GPIO_Mode = GPIO_Mode_Out_PP;
    configRx.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &configRx);

    configure_UART(115200);

    /*
     * Initializes the priorities of the interrupts
     */
    NVIC_InitTypeDef nvic_config;
	nvic_config.NVIC_IRQChannel = USART2_IRQn;
	nvic_config.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_config.NVIC_IRQChannelSubPriority = 0;
	nvic_config.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_config);

	GPIO_WriteBit(GPIOA, GPIO_Pin_0, 0); //seteo el RTS

}


//Activa o desactiva la interrupcion de USART2
inline void setInterruption(bool active){
    USART_ITConfig(USART2, USART_IT_RXNE,active?ENABLE:DISABLE);
}

//Limpia el buffer donde se almacenan los datos recibidos via interrupcion
void cleanLineBuff(){
	int i;
	atLinePos=0;
	for(i=0;i<atLineLen;i++){
		atLine[i]=0;
	}
}

static inline
char getChar()
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE == RESET));
	return USART_ReceiveData(USART2);
}

void putChar(char data)
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, data);
}

/**
 * @brief Transmit integer through UART
 *
 * Esta funciòn envìa un entero de 32 bits. El paquete consiste en 4 bytes
 * que componen el entero, un checksum y un caracter para terminar la trama.
 */
void send_number(int32_t n){
	union
	{
		int16_t integer;
		char	chars[2];
	}variable_bipolar;
	uint8_t i = 0;
	uint8_t checksum = 0;

	//transmitimos el entero por partes
	variable_bipolar.integer = n;
	putChar( variable_bipolar.chars[0] );
	putChar( variable_bipolar.chars[1] );


	//checksum para verificar integridad
	for(i=0; i < 2; i++)
		checksum += variable_bipolar.chars[i];

	putChar( checksum );
/*

	char buffer[50] = {0};
	uint8_t i = 0;
	sprintf(buffer, "%d\n", (int)n);

	while (buffer[i] != 0 )
	{
		putChar(buffer[i]);
		i++;
	}
*/
}

void send_Message_Header(uint8_t message_type){
	union
	{
		int16_t integer;
		char	chars[2];
	}variable_bipolar;
	uint8_t i = 0;
	uint8_t checksum = 0;

	//variable_bipolar.integer = 255;
	variable_bipolar.chars[0] = 0;
	variable_bipolar.chars[1] = message_type;
	putChar(variable_bipolar.chars[0]);
	putChar(variable_bipolar.chars[1]);

	/*variable_bipolar.chars[0] = 119;
	variable_bipolar.chars[1] = 85;
	variable_bipolar.chars[2] = 71;*/
	//checksum para verificar integridad
	for(i=0; i < 2; i++)
		checksum += variable_bipolar.chars[i];

	putChar( checksum );
}

void finish(){
	union
	{
		int16_t integer;
		char	chars[2];
	}variable_bipolar;

	variable_bipolar.chars[0] = 119;
	variable_bipolar.chars[1] = 85;
	variable_bipolar.chars[2] = 71;

	putChar(variable_bipolar.chars[0]);
	putChar(variable_bipolar.chars[1]);
	putChar(variable_bipolar.chars[2]);
}


void makeEcg(){

    uint32_t seconds = 30; //1*60;
    //Se deshabilitan las interrupciones
    //setInterruption(false);

    //Se envia un ack al cliente
    //putChar("a");
    startEcg(seconds);
    btState = BT_CONNECTED;

    //Se limpia el buffer para esperar nuevas ordenes
     cleanLineBuff();

    //Se habilitan las interrupciones
    //setInterruption(true);
}



/**
 * @brief Send an array through USART2
 *
 * This blocking function send a user-defined data array.
 * @param Pointer to the data to be transmitted
 * @param Number of bytes to be tranmitted
 */
static void atWrite(const char *data, uint16_t len)
{
	uint16_t i = 0;
	for (i = 0; i < len; ++i)
		putChar(data[i]);
}

// Espera OK por hasta timeout msecs. Ignora todos los datos extra que lleguen mientra espera
// Devuelve true si llega OK.
static bool atReadOK(int timeout)
{
    uint32_t finish= getMsecs() + timeout;
    bool gotOK = false;
    char c= 0;
    char cLast= 0;

    // Loop de caracteres hasta no recibir OK, considerando timeout
   while(!gotOK && getMsecs()<finish) {
        while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE)==RESET && getMsecs()<finish);
        cLast= c;

       // sleep(10);
        c= USART_ReceiveData(USART2); // Si hubo timeout vamos a recibir el ultimo char
        gotOK = cLast=='O' && c=='K';
    }

    return gotOK;
}



//
// Self testing
//
bool btTest()
{
    atWrite("AT\r", 3);
    return atReadOK(4000);
}




//Espera la orden "make ecg"
bool waitOrder(){
    while(true){
        char c;
        c= getChar();
		if(c=='m'){
			btState= BT_MAKING_ECG;
	        SH_SendString("Make Ecg.\r\n");
			return true;
		}
		if(c=='N'){
			btState= BT_DISCONNECTED;
			SH_SendString("Disconnected\r\n");
			return false;
		}
    }
    return false;
}


// Lee una linea ignorando los caracteres \r\n de prefijo, con timeout msecs
// devuelve true si no hubo errores
static bool atReadLine(char* buf, int maxLen, int timeout){
    uint32_t finish= getMsecs() + timeout;
    bool gotEndline= 0;
    bool error = false;
    char c= 0;
    char cLast= 0;
    uint16_t len= 0;

    // Loop de caracteres hasta no recibir OK, considerando timeout
    while(!gotEndline && !error && getMsecs()<finish) {
        while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE)==RESET && getMsecs()<finish);
        cLast= c;

        c= USART_ReceiveData(USART2); // Si hubo timeout vamos a recibir el ultimo char
        // Ignorar \r y \n al principio de la linea
        if(!len && (c=='\r' || c=='\n'))
            continue;

        if(len==maxLen) {
            buf[maxLen-1]= 0;
            error= true;
            printf("BT Linea muy larga.\r\n");
            continue;
        }
        buf[len]= c;

        len++;
        gotEndline= cLast=='\r' && c=='\n';
    }
    if(gotEndline && !error) {
        buf[len-2]= 0; // Si esta todo bien sacar el \r\n del final
        return true;
    } else {
        buf[0]= 0; // Sino devuelvo un string vacio
        return false;
    }
}

//Espera utilizando busy-waiting por la conexion de un cliente
bool waitForDevice(){
    while(true){
    	// Buffer para comandos y multiproposito
    //	const uint8_t cmdLen= 100;
//    	char cmd[cmdLen];

    	char c;
        c= getChar();
		if(c=='<'){
			btState= BT_CONNECTED;
	        SH_SendString("BT Connected.\r\n");
			return true;
		}
        //Mostrar el nombre del fabricante
        /*atWrite("ATI9\r", 5);
        if(atReadLine(cmd, cmdLen, 200)) {
        	SH_SendString(cmd);
        	SH_SendChar('\n');
        	if(cmd[0]=='2' || cmd[1]=='1'){
        		btState= BT_CONNECTED;
        		SH_SendString("BT Connected.\r\n");
        		return true;
        	}
        }*/
    }
    return false;
}

bool btConnect()
{
    // Buffer para comandos y multiproposito
    const uint8_t cmdLen= 100;
    char cmd[cmdLen];

    btState= BT_DISCONNECTED;

    //Warning! Este comando se ejecuta solo porque esta fallando siempre el primer comando
    atWrite("ATZ\r",4);
    atReadOK(2000);

    atWrite("ATZ\r",4);
    if(atReadOK(6000) ){
    	SH_SendString("Modulo reiniciado.\r\n");
    }

    if(!btTest()) {
            SH_SendString("[ERROR] Modulo no encontrado.\r\n");
            //return false;
        } else {
        	SH_SendString("[OK] Modulo encontrado.\r\n");
        }

    //Desactivar ECHO
    atWrite("ATE0\r", 5);
    if( atReadOK(3000) )
    	SH_SendString("[OK] BT ECHO desactivado \r\n");
    else
    	SH_SendString("[ERROR] BT ECHO ignorado \r\n");

    //Verificar el modulo
    if(!btTest()) {
    	SH_SendString("[ERROR] BT Modulo no encontrado\r\n");
        return false;
    }

    //Mostrar el nombre del fabricante
    atWrite("ATI\r", 4);
    if(!atReadLine(cmd, cmdLen, 5000)) {
    	SH_SendString("[ERROR] BT El modulo no respodio a ATI.\r\n");
        return false;
    }
    SH_SendString("[OK] BT Modulo encontrado: ");
    SH_SendString(cmd);
    SH_SendString("\n");

    //Hacer el modulo visible
    /*atWrite("AT+btp\r", 7);
    if(!atReadOK(1000)) {
        printf("[ERROR] BT Error al intentar hacer visible al modulo.\r\n");
        return false;
    }
    SH_SendString("[OK] BT Modulo visible.\r\n");*/
    SH_SendString("[OK] BT Modulo esperando conexion de cliente.\n");
    return true;
}



/*void run()
{
	setInterruption(false);
	while(1){
	  //Se espera por un cliente (busy-waiting)
	  if(btState==BT_DISCONNECTED)
		  waitForDevice();

	  if(waitOrder()){
		  makeEcg();
	  }
  }

}*/

void run()
{
	setInterruption(false);
	while(1){


	  //Se espera por un cliente (busy-waiting)
	  if(btState==BT_DISCONNECTED)
		  waitForDevice();

	  //Se activan las interrupciones
	  if(btState!=BT_DISCONNECTED)
		  setInterruption(true);

	  //Se bloquea en este punto hasta que el cliente se desconecte
	  while((btState!=BT_DISCONNECTED) && (btState!=BT_REQUESTED_ECG));
	  setInterruption(false);
	  if(btState!=BT_DISCONNECTED){
		  makeEcg();
		  SH_SendString("ECG Ready!\n");
		  /*if(cant_orders==2){
			  sec_value = sec_value + 10;
			  cant_orders=0;
		  }
		  SH_SendString("Segundos: ");
		  char buffer[50];
		  sprintf(buffer, "%d", sec_value);
		  SH_SendString(buffer);
		  SH_SendChar('\n');
		  makeEcg();
		  cant_orders++;
		  SH_SendString("ECG Ready!\n");*/
	  }
  }

}


// Configuracion persistente (ie, en la memoria no volatil) del modulo bluetooth
//WARGNING: Se utiliza solo una vez para configurar el modulo!
void btSetupModule()
{
	atWrite("ATZ\r",4);
	atReadOK(4000);

	if(!btTest()) {
        SH_SendString("Modulo no encontrado.\r\n");
        return;
    } else {
    	SH_SendString("Configurando modulo.\r\n");
    }

    //atWrite("at&f*\r", 6);
    //ok = atReadOK(1000);
    atWrite("ATE0\r", 5);
    atReadOK(1000);
    atWrite("ats520=115200\r", 14);
    atReadOK(1000);
    atWrite("at+btn=\"Portable Ecg\"\r", 22);
    atReadOK(5000);
    atWrite("ats102=1\r", 9);
    atReadOK(1000);
    atWrite("ats0=1\r", 7);
    atReadOK(1000);
    atWrite("at+btk=\"0000\"\r", 14);
    atReadOK(1000);
    atWrite("ats321=0\r", 9);
    atReadOK(1000);
    atWrite("ats512=4\r", 9);
    atReadOK(300);
    atWrite("AT&W\r", 5);
    atReadOK(500);
    printf("Reiniciando...\r\n");
    atWrite("ATZ\r", 4);
    atReadOK(5000);

    configure_UART(115200);
    /* también habría que mandar ATS520=115200 para configurar un baudrate mas rápido */
}



/**
 * @brief SART1 ISR
 *
 * This is the Interrupt Service Routine of the USART2 peripheral.
 *
 * This function is in charge of the incoming data buffering, in an
 * event-driven way.
 */
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
			char c = USART_ReceiveData(USART2); // Esta funcion tambien marca la interrupcion como atendida
			//USART_ClearFlag(USART2, USART_FLAG_RXNE);
			if((c == '\n' && atLinePos != 0) || atLinePos+1==atLineLen) {
				// Es el fin de una linea, o nos quedamos sin buffer
				atLine[atLinePos]= 0;
				// Aca podriamos hacer algo con toda la linea, pero deberia ser rapido
				atLinePos= 0;
				cleanLineBuff();
			}else
				if(c!='\r' && c!='\n' && c!='<') {
					// Si no, ir agregando caracteres en el buffer ignorando \r y \n
					atLine[atLinePos]= c;
					atLinePos++;
				}

			// Usamos los primeros 4 chars para ver que hacer
			if(atLinePos != 1)
				return;
			if(!strncmp("m", atLine, 1)) {
				if(btState != BT_REQUESTED_ECG){
					btState = BT_REQUESTED_ECG;
					SH_SendString("Make ECG.\r\n");
				}
			}else
				if(!strncmp("N", atLine, 1)) {
					btState= BT_DISCONNECTED;
					SH_SendString("BT Disconnected.\r\n");
			}
			cleanLineBuff();
	}
}
