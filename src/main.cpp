#include <QApplication>
#include <iostream>
#include <stdlib.h>
#include "include/plotus.h"

int main (int argc, char *argv[]){
    std::cout.precision(9);
    QApplication a(argc, argv);
	Plotus w;
	w.show();

	return a.exec();
}
