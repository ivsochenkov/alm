/**
 *  author:   Yuriy Lobarev
 *  telegram: @forman
 *  phone:    +7(910)983-95-90
 *  email:    forman@anyks.com
 *  site:     https://anyks.com
 */

#include <env.hpp>

/**
 * env Метод извлечения переменной окружения
 * @param  key переменная окружения
 * @return     значение переменной
 */
const std::string anyks::Env::env(const std::string & key) const noexcept {
	// Результат работы функции
	string result = "";
	// Если значение переменной передано
	if(!key.empty() && !this->prefix.empty() && (this->alphabet != nullptr)){
		// Получаем название переменной
		const string & var = (this->prefix + string("_") + this->alphabet->toUpper(key));
		// Получаем значение переменной
		const char * val = getenv(var.c_str());
		// Запоминаем результат
		result = (val == nullptr ? "" : val);
	}
	// Выводим результат
	return result;
}
/**
 * getJson Метод получения конфигурационных данных в формате JSON
 * @return конфигурационные данные
 */
const nlohmann::json anyks::Env::getJson() const noexcept {
	// Результат работы функции
	json result;
	// Если данные заполнены
	if(!this->data.empty()){
		// Значение строки в utf-8
		wstring value = L"";
		// Переходим по всему списку данных
		for(auto & item : this->data){
			// Если это флаг, устанавливаем булевое значение
			if(item.second.compare("-yes-") == 0) result[item.first] = true;
			// Если это что-то другое, устанавливаем - как есть
			else {
				// Выполняем поличение строки
				value = this->alphabet->convert(item.second);
				// Если это число или отрицательное число
				if(this->alphabet->isNumber(value) || ((value.front() == L'-') && this->alphabet->isNumber(value.substr(1)))){
					// Выполняем конвертирование в число
					result[item.first] = stoll(value);
				// Если это дробное число
				} else if(this->alphabet->isDecimal(value)) {
					// Выполняем конвертирование в дробное число
					result[item.first] = stod(value);
				// Если это массив или объект
				} else if(((item.second.front() == '{') && (item.second.back() == '}')) ||
				((item.second.front() == '[') && (item.second.back() == ']'))) {
					// Устанавливаем значение в виде объекта
					result[item.first] = json::parse(item.second);
				// Иначе добавляем значение - как оно есть
				} else result[item.first] = item.second;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * count Метод извлечения количества аргументов
 * @return количество полученных аргументов
 */
const u_short anyks::Env::count() const noexcept {
	// Выводим количество собранных аргументов
	return this->data.size();
}
/**
 * get Метод извлечения содержимого переменной окружения
 * @param  name название переменной окружения
 * @return      значение переменной окружения
 */
const char * anyks::Env::get(const std::string & name) const noexcept {
	// Результат работы функции
	const char * result = nullptr;
	// Если название переменной передано
	if(!name.empty()){
		// Ищем результат
		auto it = this->data.find(name);
		// Если переменная найдена
		if(it != this->data.end())
			// Получаем значение переменной
			result = it->second.c_str();
		// Запрашиваем данные из переменной окружения
		else {
			// Извлекаем данные переменной окружения
			this->variable = this->env(name);
			// Если переменная окружения получена
			if(!this->variable.empty()) result = this->variable.c_str();
		}
	}
	// Выводим результат
	return result;
}
/**
 * is Метод проверки наличия и значения переменной
 * @param key ключ - название переменной для проверки
 * @param val значение переменной для проверки (если требуется)
 */
const bool anyks::Env::is(const std::string & key, const std::string & val) const noexcept {
	// Результат работы функции
	bool result = false;
	// Если ключ передан
	if(!key.empty()){
		// Ищем результат
		auto it = this->data.find(key);
		// Если переменная найдена
		if(it != this->data.end()){
			// Формируем результат
			result = (
				val.empty() ? true :
				this->alphabet->toLower(it->second)
				.compare(this->alphabet->toLower(val)) == 0
			);
		}
	}
	// Выводим результат
	return result;
}
/**
 * autoRead Метод разрешения или запрещения автоматического чтения текстовой переменной
 * @param flag значение флага
 */
void anyks::Env::autoRead(const bool flag) noexcept {
	// Устанавливаем флаг автоматического чтения текстовой переменной
	this->aread = flag;
}
/**
 * setJson Метод добавления данных в формате json
 * @param data данные в формате json
 */
void anyks::Env::setJson(const json & data) noexcept {
	// Если данные переданы
	if(!data.empty()){
		// Ключ и значение ключа
		string key = "", val = "";
		// Переходим по всем ключам и добавляем всё в базу данных
		for(auto & el : data.items()){
			// Получаем ключ
			key = el.key();
			// Если это пустой объект
			if(data.at(key).is_null()) continue;
			// Если это булевое значение
			if(data.at(key).is_boolean()){
				// Если это положительное булевое значение
				if(el.value()) val = "-yes-";
				// Если это отрицательное булевое значение
				else continue;
			// Если это чило
			} else if(data.at(key).is_number()) val = to_string(el.value());
			// Если это массив или объект
			else if(data.at(key).is_object() || data.at(key).is_array()) val = data.at(key).dump();
			// Иначе это обычная строка
			else val = el.value();
			// Добавляем значение ключа в базу параметров
			if(!val.empty()) this->data.emplace(key, val);
		}
	}
}
/**
 * textEnv Метод установки списка имён которые нужно извлечь
 * @param text название переменной для извлечения текстовой информации из потока (если параметром не передана)
 */
void anyks::Env::textEnv(const string & text) noexcept {
	// Если название текстовой переменной передано
	if(!text.empty()) this->text = move(text);
}
/**
 * prefixEnv Метод установки префикса переменной окружения
 * @param prefix префикс переменной окружения
 */
void anyks::Env::prefixEnv(const string & prefix) noexcept {
	// Если префикс передан, устанавливаем его
	if(!prefix.empty() && (this->alphabet != nullptr)){
		// Устанавливаем префикс переменной
		this->prefix = this->alphabet->toUpper(prefix);
	}
}
/**
 * setAlphabet Метод установки алфавита
 * @param alphabet объект алфавита
 */
void anyks::Env::setAlphabet(const alphabet_t * alphabet) noexcept {
	// Если алфавит передан
	if(alphabet != nullptr) this->alphabet = alphabet;
}
/**
 * read Метод чтения данных переменных окружения
 * @param args  список переменных окружения
 * @param count количество переменных окружения
 */
void anyks::Env::read(const char * args[], const u_short count) noexcept {
	// Позиция найденного значения
	size_t pos = 0;
	// Требуется прочитать значение
	bool isValue = false;
	// Строка аргумента, ключ и занчение
	string arg = "", key = "", val = "";
	// Переходим по массиву аргументов
	for(u_short i = 0; i < count; i++){
		// Получаем значение аргумента
		arg = args[i];
		// Если это прямое значение переменной
		if(arg.substr(0, 2).compare("--") == 0){
			// Убираем ожидание значения
			isValue = false;
			// Ищем знак равенства
			pos = arg.find("=", 2);
			// Если знак равенства найден
			if(pos != string::npos){
				// Получаем ключ
				key = arg.substr(2, pos - 2);
				// Получаем значение
				val = arg.substr(pos + 1, arg.length() - (pos + 1));
			// Если знак равенства не найден
			} else {
				// Очищаем значение
				val.clear();
				// Получаем ключ
				key = arg.substr(2, arg.length() - 2);
			}
			// Добавляем полученные данные в список переменных
			if(!key.empty()) this->data.emplace(key, val);
		// Если это относительное значение переменной
		} else if(arg.front() == '-') {
			// Получаем ключ
			key = arg.substr(1, arg.length() - 1);
			// Устанавливаем полученное значение
			this->data.emplace(key, "-yes-");
			// Устанавливаем ожидание значения
			isValue = true;
		// Если это ожидание значения
		} else if(isValue) {
			// Убираем ожидание значения
			isValue = false;
			// Добавляем полученные данные в список переменных
			if(!key.empty() && (this->data.count(key) > 0)) this->data.at(key) = arg;
		}
	}
	// Если переменная текста установлена и мы её из не получили
	if(!this->text.empty() && (this->data.count(this->text) < 1)){
		// Очищаем значение
		val.clear();
		// Считываем строку из буфера stdin
		if(!isatty(STDIN_FILENO) || (this->aread && this->env(this->text).empty())) 
            std::getline(std::cin, val);
		// Добавляем полученные данные в список переменных
		if(!val.empty()) this->data.emplace(this->text, move(val));
	}
}
/**
 * Env Конструктор
 * @param prefix префикс переменной окружения
 */
anyks::Env::Env(const std::string & prefix) noexcept {
	// Если префикс передан, устанавливаем его
	if(!prefix.empty()) this->prefixEnv(prefix);
}
/**
 * Env Конструктор
 * @param alphabet объект алфавита
 */
anyks::Env::Env(const alphabet_t * alphabet) noexcept {
	// Устанавливаем переданный алфавит
	this->setAlphabet(alphabet);
}
/**
 * Env Конструктор
 * @param prefix префикс переменной окружения
 * @param text   название переменной для извлечения текстовой информации из потока (если параметром не передана)
 */
anyks::Env::Env(const std::string & prefix, const std::string & text) noexcept {
	// Если текст передан, устанавливаем его
	if(!text.empty()) this->textEnv(text);
	// Если префикс передан, устанавливаем его
	if(!prefix.empty()) this->prefixEnv(prefix);
}
/**
 * Env Конструктор
 * @param prefix   префикс переменной окружения
 * @param alphabet объект алфавита
 */
anyks::Env::Env(const std::string & prefix, const alphabet_t * alphabet) noexcept {
	// Устанавливаем переданный алфавит
	this->setAlphabet(alphabet);
	// Если префикс передан, устанавливаем его
	if(!prefix.empty()) this->prefixEnv(prefix);
}
/**
 * Env Конструктор
 * @param prefix   префикс переменной окружения
 * @param text     название переменной для извлечения текстовой информации из потока (если параметром не передана)
 * @param alphabet объект алфавита
 */
anyks::Env::Env(const string & prefix, const std::string & text, const alphabet_t * alphabet) noexcept {
	// Устанавливаем переданный алфавит
	this->setAlphabet(alphabet);
	// Если текст передан, устанавливаем его
	if(!text.empty()) this->textEnv(text);
	// Если префикс передан, устанавливаем его
	if(!prefix.empty()) this->prefixEnv(prefix);
}
