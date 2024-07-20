/*
All device drivers are required to support these functions
*/

#define check_classcode(x) (functions[0])(x)
#define attach_model(x) (functions[1])(x)       //this one gives the passed structure the pointer to the functions on its own, on fail it returns something in data idk
#define irby_model_and_pin(x) (functons[2](x))
