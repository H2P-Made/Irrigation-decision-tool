/*
 * ============================================================
 * Irrigation Flow Rate Calculator (Refined)
 * Language : C++17
 * Purpose  : Calculate irrigation flow rates and water volumes
 * with improved buffer handling and safety checks.
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>
#include <map>
#include <algorithm>

// ── ANSI colour helpers ──────────────────────────────────────
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

// ── Constants ───────────────────────────────────────────────
constexpr double MM_PER_M     = 1000.0;
constexpr double HA_TO_M2     = 10000.0;
constexpr double MINS_PER_HR  = 60.0;
constexpr double SECS_PER_MIN = 60.0;

// ── Data structures ─────────────────────────────────────────
struct IrrigationResult {
    double flow_rate_lps;       
    double flow_rate_m3ph;      
    double total_volume_m3;     
    double application_time_hr; 
    double peak_demand_lps;     
};

// ── Utility: safe numeric input ─────────────────────────────
double getPositiveDouble(const std::string& prompt, double min_val = 0.0001, double max_val = 1e12) {
    double value;
    while (true) {
        std::cout << CYAN << "  " << prompt << RESET << " ";
        if (std::cin >> value && value >= min_val && value <= max_val) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << YELLOW << "  ⚠  Invalid input. Enter a value between " << min_val << " and " << max_val << "\n" << RESET;
    }
}

int getIntChoice(const std::string& prompt, int min_v, int max_v) {
    int value;
    while (true) {
        std::cout << CYAN << "  " << prompt << RESET << " ";
        if (std::cin >> value && value >= min_v && value <= max_v) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << YELLOW << "  ⚠  Please enter [" << min_v << "-" << max_v << "]\n" << RESET;
    }
}

void divider(char ch = '─', int len = 56) {
    std::cout << std::string(len, ch) << "\n";
}

// ── FORMULAS ────────────────────────────────────────────────
// Manning's formula for rectangular open-channel flow: $Q = \frac{1}{n} A R^{2/3} S^{1/2}$
double manningsFlow(double n, double width_m, double depth_m, double slope) {
    double area      = width_m * depth_m;
    double perimeter = width_m + (2.0 * depth_m);
    double R         = area / perimeter;
    double Q_m3s     = (1.0 / n) * area * std::pow(R, 2.0 / 3.0) * std::sqrt(slope);
    return Q_m3s * 1000.0; // L/s
}

double emitterFlow_Lph(double Kd, double pressure_kPa, double x) {
    return Kd * std::pow(pressure_kPa, x);
}

double fieldWaterVolume_m3(double ETc_mm, double Pe_mm, double area_ha, double Ea) {
    double net_mm = std::max(0.0, ETc_mm - Pe_mm);
    return (net_mm / MM_PER_M) * (area_ha * HA_TO_M2) / Ea;
}

double flowFromVolume_Lps(double volume_m3, double hours) {
    double secs = hours * MINS_PER_HR * SECS_PER_MIN;
    return (volume_m3 * 1000.0) / secs;
}

// ── DISPLAY ─────────────────────────────────────────────────
void printResult(const IrrigationResult& r) {
    divider('═');
    std::cout << BOLD << GREEN << "  📊  CALCULATION RESULTS\n" << RESET;
    divider('─');
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Flow Rate         : " << BOLD << r.flow_rate_lps    << RESET << " L/s\n";
    std::cout << "  Flow Rate         : " << BOLD << r.flow_rate_m3ph   << RESET << " m³/hr\n";
    std::cout << "  Total Volume      : " << BOLD << r.total_volume_m3  << RESET << " m³\n";
    std::cout << "  Application Time  : " << BOLD << r.application_time_hr << RESET << " hrs\n";
    std::cout << "  Peak Demand (1.15): " << BOLD << r.peak_demand_lps  << RESET << " L/s\n";
    divider('═');
}

// ── MODULES ─────────────────────────────────────────────────
void runMannings() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── Manning's (Rectangular Channel) ──\n" << RESET;
    
    std::map<int, std::pair<std::string, double>> roughness = {
        {1, {"Concrete-lined canal",    0.013}},
        {2, {"Earth channel (clean)",   0.022}},
        {3, {"Earth with vegetation",   0.035}},
        {4, {"Gravel / riprap channel", 0.030}},
    };
    for (auto const& [k, v] : roughness)
        std::cout << "    [" << k << "] " << v.first << " (n = " << v.second << ")\n";

    int choice  = getIntChoice("Choice:", 1, 4);
    double n    = roughness[choice].second;
    double W    = getPositiveDouble("Channel width (m):");
    double D    = getPositiveDouble("Water depth (m):");
    double S    = getPositiveDouble("Bed slope (m/m, e.g., 0.001):", 0.00001);

    double Q_Lps   = manningsFlow(n, W, D, S);
    double Q_m3ph  = Q_Lps * 3.6;

    double area_ha = getPositiveDouble("Field area to irrigate (ha):");
    double app_mm  = getPositiveDouble("Target application depth (mm):");
    double V_m3    = (app_mm / MM_PER_M) * area_ha * HA_TO_M2;
    double T_hr    = (Q_m3ph > 0) ? (V_m3 / Q_m3ph) : 0;

    IrrigationResult r;
    r.flow_rate_lps    = Q_Lps;
    r.flow_rate_m3ph   = Q_m3ph;
    r.total_volume_m3  = V_m3;
    r.application_time_hr = T_hr;
    r.peak_demand_lps  = Q_Lps * 1.15;
    printResult(r);
}

void runDrip() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── Drip Irrigation System ──\n" << RESET;
    double Kd      = getPositiveDouble("Emitter coefficient Kd (e.g., 0.5):");
    double H       = getPositiveDouble("Operating pressure (kPa):");
    double x       = getPositiveDouble("Emitter exponent x (0.5 for turbulent):");
    double spacing = getPositiveDouble("Emitter spacing (m):");
    double rows    = getPositiveDouble("Number of rows:");
    double rowLen  = getPositiveDouble("Row length (m):");

    double q_Lph       = emitterFlow_Lph(Kd, H, x);
    long   totalEmit   = static_cast<long>(rows * (rowLen / spacing));
    double totalFlow_Lph = q_Lph * totalEmit;
    
    double ETc  = getPositiveDouble("Crop ETc (mm/day):");
    double Pe   = getPositiveDouble("Effective rainfall Pe (mm/day):", 0.0);
    double area = getPositiveDouble("Field area (ha):");
    double Ea   = getPositiveDouble("Efficiency Ea (decimal, e.g., 0.90):", 0.1, 1.0);

    double V   = fieldWaterVolume_m3(ETc, Pe, area, Ea);
    double T   = (totalFlow_Lph > 0) ? (V / (totalFlow_Lph / 1000.0)) : 0;

    IrrigationResult r;
    r.flow_rate_lps    = totalFlow_Lph / 3600.0;
    r.flow_rate_m3ph   = totalFlow_Lph / 1000.0;
    r.total_volume_m3  = V;
    r.application_time_hr = T;
    r.peak_demand_lps  = r.flow_rate_lps * 1.15;

    std::cout << "  " << YELLOW << "Total emitters: " << totalEmit << " | Per-emitter: " 
              << std::fixed << std::setprecision(2) << q_Lph << " L/hr" << RESET << "\n";
    printResult(r);
}

void runFieldDemand() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── General Field Water Demand ──\n" << RESET;
    double ETc  = getPositiveDouble("Crop ETc (mm/day):");
    double Pe   = getPositiveDouble("Effective rainfall (mm/day):", 0.0);
    double area = getPositiveDouble("Field area (ha):");
    double Ea   = getPositiveDouble("System efficiency (decimal, e.g., 0.75):", 0.1, 1.0);
    double hrs  = getPositiveDouble("Daily operating hours:");

    double V      = fieldWaterVolume_m3(ETc, Pe, area, Ea);
    double Q_Lps  = flowFromVolume_Lps(V, hrs);

    IrrigationResult r;
    r.flow_rate_lps    = Q_Lps;
    r.flow_rate_m3ph   = Q_Lps * 3.6;
    r.total_volume_m3  = V;
    r.application_time_hr = hrs;
    r.peak_demand_lps  = Q_Lps * 1.15;
    printResult(r);
}

// ── MAIN ────────────────────────────────────────────────────
int main() {
    std::cout << "\n";
    divider('═');
    std::cout << BOLD << GREEN << "       💧  IRRIGATION FLOW RATE CALCULATOR\n" << RESET;
    divider('═');

    bool running = true;
    while (running) {
        std::cout << "\n  " << BOLD << "Select Calculation Method:\n" << RESET;
        std::cout << "    [1]  Open-Channel Flow  (Manning's)\n";
        std::cout << "    [2]  Drip Irrigation    (Emitter discharge)\n";
        std::cout << "    [3]  Field Water Demand (ETc-based)\n";
        std::cout << "    [0]  Exit\n\n";

        int sel = getIntChoice("Your choice [0-3]:", 0, 3);
        if (sel == 0) {
            running = false;
        } else {
            switch (sel) {
                case 1: runMannings();    break;
                case 2: runDrip();        break;
                case 3: runFieldDemand(); break;
            }
            std::cout << "\n" << CYAN << "  Press ENTER to return to menu..." << RESET;
            std::cin.get(); 
        }
    }

    std::cout << GREEN << BOLD << "\n  👋  Calculations complete. Goodbye!\n\n" << RESET;
    return 0;
}
