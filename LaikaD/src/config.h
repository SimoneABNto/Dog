#ifndef _CONFIG_H
#define _CONFIG_H

#define VERSION "v0.1.12"

//configurazione motore

#define MAIN_MOTOR_CURRENT 1000          //mA
#define MAIN_MOTOR_STEP_PER_ROTATION 200 //steps
#define MAIN_MOTOR_MICROSTEP 16          //1/x in questo caso --> ogni 16 step software equivale uno reale del motore

// this value is used to calibrate the cell
// change this value while the weight of the cell correspond to the output
#define LOAD_CELL_ADJ_FULL_SCALE 380 

// pulse per rotation of the encoder
#define ENCODER_MAIN_PULSE_PER_ROTATION 2400

// main motor doser configs
#define MAIN_MOTOR_MAX_ROTATION_PER_MIN 20  //rotation*min
#define MAIN_MOTOR_MIN_ROTATION_PER_MIN 10   //rotation*min
#define MAIN_MOTOR_FINAL_ROTATION_PER_MIN 6 //rotation*min

//valore in grammi, ultimi grami in cui il motore gira alla velocit� minima
#define WEIGHT_FOR_FINAL_SPEED 30 //grams

//ogni quanti milliseconsd viene controllata la cella di carico durante drop()
#define LOAD_CELL_TIME_CECK 400 //milliseconds
//valore entro il quale la cella di carico pu� misurare un valore inferiore all'ultimo misurato. Per evitare il rimbalzo.
//#define LOAD_CELL_OSCILLATION_VALUE			6		//grams  attualmente non utilizzato

// move the moto backwards on end to avoid more food drop on vibration
#define MAIN_MOTOR_BACKWARDS_ON_END 20 //steps

//in step reali del motore  (se il motore � da 200 step allora 50 = 1/4 di giro)
//quanto il motore pu� tornare in dietro se si blocca
#define MAIN_MOTOR_STD_BACKWARDS_ROTATION 20 //steps
//differenza massima tra encoder e steps contati per avviare il ritorno indietro
#define MAX_DIF_ENCODER_STEPPER 5 //steps
//in gram, peso massimo che pu� contenere il WEIGHING_COLLECTOR oltre il quale bisogna dividere la pesata
#define MAX_WEIGHT_IN_WEIGHING_COLLECTOR 110 //grams
//peso sopra al peso massimo che permette di eseguire una nuova pesata.
//se ci sono pochi grammi sopra MAX_WEIGHT_IN_WEIGHING_COLLECTOR la pesata multipla non pu� essere precisa
#define OVER_WEIGHT_IN_WEIGHING_COLLECTOR 25 //grams

//definisce lo stato aperto o chiuso delle porte, sulla base della logica della scheda NO/NC
#define SWITCH_DOOR_OPEN LOW
#define SWITCH_DOOR_CLOSE HIGH

//SERVO
#define SERVO_MOTOR_MAX_ANGLE 180 //grade
#define SERVO_MOTOR_MIN_ANGLE 0   //grade

#define SERVO_MOTOR_SLOW_OPEN_SPEED 25 //deg*sec
#define SERVO_MOTOR_FAST_OPEN_SPEED 50 //deg*sec
//range in gradi dentro il quale il motore si muove a SERVO_MOTOR_SLOW_OPEN_SPEED
//fuori dal range invece si muove a SERVO_MOTOR_FAST_OPEN_SPEED
#define SERVO_MOTOR_IN_GRAD_SLOW_SPEED 30  //deg
#define SERVO_MOTOR_OUT_GRAD_SLOW_SPEED 70 //deg

#define SERVO_MOTOR_CLOSE_SPEED 80 //deg*sec
//tempo per chiudere in ceck_trapdoor_closed()
#define TIME_TO_INITIAL_CLOSE_TRAPDOOR 200 //milliseconds

//la somma di tutti i timeout deve essere inferiore o uguale a FEED_TIMEOUT
//tempo complessivo per dare da mangiare
#define FEED_TIMEOUT 1500000    //milliseconds
#define WEIGHING_TIMEOUT 50000 //milliseconds
//soglia sotto al quale parte l'avvertimento per il quasi esaurimento del cibo
#define STATIC_WARNING_THRESHOLD_LOW_FOOD 5000 //grams

#define DEBUG_SERIAL_PRINT_ON true

// disable or enable automatic food erogation
// with false only manual erogation will be avaiable
#define SCHEDULED_AUTO_EROGATION_ENABLED true

#endif
