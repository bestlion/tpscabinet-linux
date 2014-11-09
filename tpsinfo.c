#include "tpsinfo.h"

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	char *tpslogin;
	char *tpspass;

	if (argc == 3) {
		tpslogin = argv[1];
		tpspass = argv[2];
	}
	else if (argc == 1 && strlen(TPSLOGIN) != 0 && strlen(TPSPASSWORD) != 0) {
		tpslogin = TPSLOGIN;
		tpspass = TPSPASSWORD;
	}
	else {
		usage();
		exit(2);
	}

	char *url = "https://cabinet.tps.uz/login";
	CURLcode CURLStat;
	/// gurl global inint
	CURLStat = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(CURLStat != CURLE_OK) {
		fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(CURLStat));
		exit(1);
	}
	/// declare post vars
	char *tpl = "LoginForm[username]=%s&LoginForm[password]=%s&yt0=submit";
	char *postvars;
	postvars = (char *)malloc(strlen(tpl)+strlen(tpslogin)+strlen(tpspass)-4+1); // -4+1 is to consider the %s %s in tpl and the ending \0
	sprintf(postvars, tpl, tpslogin, tpspass);
	
	ResponseStruct response;
	CabinetInfoStruct CabinetInfo = {.TraffUsed=0, .TraffLeft=0, .Balance=0, .CabinetID=0, .AbonentPlata=0, .Kurs=0};
	
	char *result;

	/// while()//
	response.data = malloc(1);
	response.size = 0;
	/// do request (login and get data)
	CURLStat = do_web_request(url, postvars, &response);
	if (CURLStat == CURLE_OK) {
		if (strlen(response.data)!=0) {
			if (GetCabinetVal(response.data, "Вход в персональный кабинет", &result)==0)
				fprintf(stderr, "tpsinfo: Логин\\пароль не верны\n");
			else {
				//////all ok start parsing///////////
				if (GetCabinetVal(response.data, "traffic[ ]*:[ ]*'Использовано'[ ]*,[ ]*value:[ ]*([-]?[0-9\\.]+)", &result)==0) { CabinetInfo.TraffUsed = atof(result); free(result);} else CabinetInfo.TraffUsed = 0;
				if (GetCabinetVal(response.data, "traffic[ ]*:[ ]*'Осталось'[ ]*,[ ]*value:[ ]*([-]?[0-9\\.]+)", &result)==0) { CabinetInfo.TraffLeft = atof(result); free(result);} else CabinetInfo.TraffLeft = 0;
				if (CabinetInfo.TraffLeft < 0) CabinetInfo.TraffLeft = 0;
				if (GetCabinetVal(response.data, "class=\"balance\"[ ]*data-accid=\"[0-9]+\">[ ]*([-]?[0-9\\.]+)", &result)==0) { CabinetInfo.Balance = atof(result); free(result);} else CabinetInfo.Balance = 0;
				if (GetCabinetVal(response.data, "/pppoe_session\\?id=([0-9]+)", &result)==0) { CabinetInfo.CabinetID = atoi(result); free(result);} else CabinetInfo.CabinetID = 0;
				if (GetCabinetVal(response.data, "Статус</th>[^\\$]+<td>[ ]*\\$[ ]*([0-9\\.]+)", &result)==0) { CabinetInfo.AbonentPlata = atof(result); free(result);} else CabinetInfo.AbonentPlata = 0;
				if (GetCabinetVal(response.data, "доллар[ ]*США[ ]*=[ ]*([0-9\\.]+)", &result)==0) { CabinetInfo.Kurs = atof(result); free(result);} else CabinetInfo.Kurs = 0;
				ShowInfo(&CabinetInfo);
			}
		}
		else {
			fprintf(stderr, "tpsinfo (Network error): Данные не получены\n");
		}
		/////////////////////////////////////
	}
	else {
		fprintf(stderr, "Curl error: %s\n", curl_easy_strerror(CURLStat));
	}
	free(response.data);
	response.data=NULL;
	/// end while()//

	result = NULL;
	free(postvars); postvars=NULL;
	tpl=NULL; url=NULL; tpslogin=NULL; tpspass=NULL;
	curl_global_cleanup();
	exit(0);
}

void ShowInfo(CabinetInfoStruct *CabinetInfo) {
	time_t rawtime = time(NULL);
	struct tm timeinfo = *localtime(&rawtime);	
	
	float DayUsed = CabinetInfo->TraffUsed / timeinfo.tm_mday;
	float PlaningDayUse = 0;
	if ((DaysInMonth(timeinfo.tm_mon+1, timeinfo.tm_year+1900) - timeinfo.tm_mday) > 0)
		PlaningDayUse = CabinetInfo->TraffLeft / (DaysInMonth(timeinfo.tm_mon+1, timeinfo.tm_year+1900) - timeinfo.tm_mday);

	printf("Использовано: %.2f Мб", CabinetInfo->TraffUsed);
	if (DayUsed>0)
		printf(" (%.2f Мб/день)", DayUsed);
	printf("\n");
	printf("Остаток: %.2f Мб", CabinetInfo->TraffLeft);
	if (PlaningDayUse>0)
		printf(" (%.2f Мб/день)", PlaningDayUse);
	printf("\n");
    printf("Баланс: $%.2f / Курс: %.2f\n", CabinetInfo->Balance, CabinetInfo->Kurs);
	printf("К оплате (1-го): %.2f\n", (CabinetInfo->AbonentPlata - CabinetInfo->Balance) * CabinetInfo->Kurs);
}

int DaysInMonth(int month, int year) {
	if (month == 2)
		return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))?29:28;//is Leap Year
	else
		return ((month == 4 || month == 6 || month == 9 || month == 11))?30:31;
}

int GetCabinetVal(const char *str, char *pattern, char **result) {
        int status;
        regex_t preg;
		size_t     nmatch = 2;
		regmatch_t* matches = malloc(sizeof(regex_t) * nmatch);
		status = regcomp(&preg, pattern, REG_ICASE|REG_EXTENDED);
		if (status == 0) {
			status = regexec(&preg, str, nmatch, matches, 0);
			if (status == 0) {
				*result = (char*)malloc(matches[1].rm_eo - matches[1].rm_so);
				strncpy(*result, &str[matches[1].rm_so], matches[1].rm_eo - matches[1].rm_so);
			}
		}
		free(matches);
		regfree(&preg);
		pattern = NULL;////
		return status;
}

void usage()
{
fprintf(stderr, "USAGE: tpsinfo [login] [password]\n\
\tlogin: login for TPS cabinet ex: jetXXXXXXX\n\
\tpassword: password for TPS cabinet\n");
}

CURLcode do_web_request(char *url, char *postvars, ResponseStruct *response)
{
    CURL *curl_handle = NULL;
	CURLcode res;
	curl_handle = curl_easy_init();
	if (curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.41 Safari/534.7"); /// google chrome
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)response);
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1L);
	
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		//curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long) strlen(postvars));
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postvars);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");

		res = curl_easy_perform(curl_handle);
		curl_easy_cleanup(curl_handle);
	}
	else
		res = CURLE_FAILED_INIT;
	return res;
}
//

size_t WriteMemoryCallback(void *buffer, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  ResponseStruct *response = (ResponseStruct *)userp;
  response->data = realloc(response->data, response->size + realsize + 1);
  if(response->data == NULL) { /// out of memory
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  memcpy(&(response->data[response->size]), buffer, realsize);
  response->size += realsize;
  response->data[response->size] = 0;
  return realsize;
}
