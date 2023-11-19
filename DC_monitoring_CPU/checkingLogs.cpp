#include <iostream>
#include <vector>
#include <string>

struct LogEntry {
    std::string timestamp;
    std::string event_type;
    std::string duration;
};

class LogController {
private:
    std::vector<LogEntry> log_entries;

public:
    void addLogEntry(const std::string& timestamp, const std::string& event_type, const std::string& duration) {
        log_entries.push_back({timestamp, event_type, duration});
    }

    std::vector<LogEntry> listEventsInDateRange(const std::string& start_date, const std::string& end_date) {
        std::vector<LogEntry> filtered_entries;
        for (const auto& entry : log_entries) {
            if (entry.timestamp >= start_date && entry.timestamp <= end_date) {
                filtered_entries.push_back(entry);
            }
        }
        return filtered_entries;
    }

    std::pair<int, int> totalActiveTimeInDateRange(const std::string& start_date, const std::string& end_date) {
        int total_minutes = 0;
        for (const auto& entry : log_entries) {
            if (entry.timestamp >= start_date && entry.timestamp <= end_date) {
                // Converte a duração de horas e minutos para minutos e soma ao tempo total
                int hours, minutes;
                sscanf(entry.duration.c_str(), "%d:%d", &hours, &minutes);
                total_minutes += hours * 60 + minutes;
            }
        }

        return {total_minutes / 60, total_minutes % 60};
    }
};

int main() {
    LogController logController;

    // Adiciona algumas entradas de log para teste
    logController.addLogEntry("2023-01-01 12:00", "EVNT", "02:30");
    logController.addLogEntry("2023-01-02 15:30", "EVNT", "01:45");
    logController.addLogEntry("2023-01-03 08:45", "EVNT", "03:15");

    // Realiza consultas
    std::string startDate = "2023-01-01";
    std::string endDate = "2023-01-03";

    std::vector<LogEntry> eventsInDateRange = logController.listEventsInDateRange(startDate, endDate);
    std::cout << "Eventos no intervalo de datas:" << std::endl;
    for (const auto& entry : eventsInDateRange) {
        std::cout << entry.timestamp << " | " << entry.event_type << " | " << entry.duration << std::endl;
    }

    auto totalActiveTime = logController.totalActiveTimeInDateRange(startDate, endDate);
    std::cout << "\nTempo total ativo no intervalo de datas: " << totalActiveTime.first << " horas e " << totalActiveTime.second << " minutos" << std::endl;

    return 0;
}
