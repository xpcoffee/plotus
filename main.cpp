#include "bareminimumplotter.h"
#include <QApplication>
#include <iostream>
#include <stdlib.h>


int main (int argc, char *argv[]){
    std::cout.precision(9);
    QApplication a(argc, argv);
	BareMinimumPlotter w;
	w.show();

	return a.exec();
}
