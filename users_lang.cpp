#include "users_lang.h"


Users_Lang::Users_Lang(QObject *parent) : QObject(parent)
{
 
}

Users_Lang::~Users_Lang()
{
    qDebug("By Users_Lang!");
}

QString Users_Lang::get_p(int n){

	if(lng==0) return lang_ru[n];
	else if (lng==1) return lang_en[n];
	
    return "";
}

int Users_Lang::set_lang(QString lang){

  if(lang == "ru"){
    lng = 0;
	return 1;
  }
  if(lang == "en"){
    lng = 1;
	return 1;
  }

  return 0;
}
