#ifndef UPDATE_0072_H
#define UPDATE_0072_H

#include <iupdate.h>

class Update0072 : public IUpdate {
	
	public:
		virtual QString from_version();
		virtual QString version();
		virtual QString description();
		virtual void update(QSqlDatabase &db);
};

#endif // UPDATE_0072_H
