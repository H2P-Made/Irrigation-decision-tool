import java.util.*;
import java.io.*;
import java.time.*;
import java.time.format.*;
import java.util.stream.*;

// ── Domain model ─────────────────────────────────────────────
class CropRecord {
    private static int counter = 1;

    final int    id;
    final String date;
    String cropName;
    String fieldName;
    double areaHectares; 
    String soilType;
    double irrigationMm;
    double fertilizerKgPerHa; 
    String notes;

    CropRecord(String cropName, String fieldName, double areaHa,
               String soilType, double irrigationMm, double fertilizerKg,
               String notes) {
        this.id            = counter++;
        this.date          = LocalDate.now().format(DateTimeFormatter.ofPattern("dd-MMM-yyyy"));
        this.cropName      = cropName;
        this.fieldName     = fieldName;
        this.areaHectares  = areaHa;
        this.soilType      = soilType;
        this.irrigationMm  = irrigationMm;
        this.fertilizerKgPerHa = fertilizerKg;
        this.notes         = notes;
    }

    CropRecord(int id, String date, String crop, String field, double area, String soil, double irrig, double fert, String notes) {
        this.id = id;
        this.date = date;
        this.cropName = crop;
        this.fieldName = field;
        this.areaHectares = area;
        this.soilType = soil;
        this.irrigationMm = irrig;
        this.fertilizerKgPerHa = fert;
        this.notes = notes;
        if (id >= counter) counter = id + 1; 
    }

    double irrigationVolumeLiters() {
        return irrigationMm * areaHectares * 10000;
    }

    double totalFertilizerMass() {
        return fertilizerKgPerHa * areaHectares;
    }

    @Override
    public String toString() {
        return String.format(
            "  #%-3d | %-12s | %-10s | %6.2f ha | %-8s | %5.1f mm | %6.1f kg/ha",
            id, cropName, fieldName, areaHectares, soilType, irrigationMm, fertilizerKgPerHa
        );
    }
}

class C {
    static final String RESET   = "\033[0m";
    static final String BOLD    = "\033[1m";
    static final String GREEN   = "\033[32m";
    static final String CYAN    = "\033[36m";
    static final String YELLOW  = "\033[33m";
    static final String RED     = "\033[31m";
    static final String PURPLE  = "\033[35m";
}

// CRITICAL: File MUST be named FarmDataApp.java
public class FarmDataApp {
    private final List<CropRecord> records = new ArrayList<>();
    private final Scanner sc = new Scanner(System.in);
    private final String DATA_FILE = "farm_data_master.csv";

    // This is the "Main String" method the compiler was looking for
    public static void main(String[] args) {
        new FarmDataApp().run();
    }

    void run() {
        loadFromCSV(); 
        printBanner();
        boolean active = true;
        while (active) {
            printMenu();
            int choice = readInt("Select option", 0, 7);
            switch (choice) {
                case 1 -> addRecord();
                case 2 -> viewAllRecords();
                case 3 -> searchRecords();
                case 4 -> showSummary();
                case 5 -> editRecord();
                case 6 -> deleteRecord();
                case 7 -> saveToCSV(true); 
                case 0 -> {
                    saveToCSV(false); 
                    active = false;
                }
            }
        }
        System.out.println(C.GREEN + C.BOLD + "\n  🌾  Data saved. Goodbye!\n" + C.RESET);
    }

    void saveToCSV(boolean verbose) {
        try (PrintWriter pw = new PrintWriter(new FileWriter(DATA_FILE))) {
            pw.println("ID,Date,Crop,Field,Area,Soil,Irrig,Fert,Notes");
            for (CropRecord r : records) {
                pw.printf("%d,%s,%s,%s,%.2f,%s,%.1f,%.1f,%s%n",
                    r.id, r.date, r.cropName.replace(",", ";"), 
                    r.fieldName.replace(",", ";"), r.areaHectares, 
                    r.soilType, r.irrigationMm, r.fertilizerKgPerHa, 
                    r.notes.replace(",", ";"));
            }
            if (verbose) System.out.println(C.GREEN + "  ✅  Data exported to " + DATA_FILE + C.RESET);
        } catch (IOException e) {
            System.out.println(C.RED + "  ❌  Save failed: " + e.getMessage() + C.RESET);
        }
    }

    void loadFromCSV() {
        File file = new File(DATA_FILE);
        if (!file.exists()) return;
        try (BufferedReader br = new BufferedReader(new FileReader(file))) {
            br.readLine(); 
            String line;
            while ((line = br.readLine()) != null) {
                String[] p = line.split(",", -1); 
                if (p.length < 9) continue;
                records.add(new CropRecord(
                    Integer.parseInt(p[0]), p[1], p[2], p[3], 
                    Double.parseDouble(p[4]), p[5], Double.parseDouble(p[6]), 
                    Double.parseDouble(p[7]), p[8]
                ));
            }
        } catch (Exception e) {
            System.out.println(C.YELLOW + "  ⚠️  Starting with fresh data." + C.RESET);
        }
    }

    void addRecord() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  📋  ADD RECORD" + C.RESET);
        String crop = readString("Crop name");
        String field = readString("Field name");
        double area = readDouble("Area (ha)", 0.01, 1000);
        String soil = readString("Soil Type");
        double irrig = readDouble("Irrigation (mm)", 0, 500);
        double fert = readDouble("Fertilizer (kg/ha)", 0, 1000);
        String notes = readOptional("Notes");
        
        records.add(new CropRecord(crop, field, area, soil, irrig, fert, notes.isEmpty() ? "N/A" : notes));
        System.out.println(C.GREEN + "  ✅  Added!" + C.RESET);
    }

    void viewAllRecords() {
        if (records.isEmpty()) { noDataMsg(); return; }
        System.out.println("\n" + C.BOLD + "  ID  | CROP         | FIELD      | AREA      | SOIL     | WATER    | FERT" + C.RESET);
        records.forEach(System.out::println);
    }

    void searchRecords() {
        if (records.isEmpty()) { noDataMsg(); return; }
        String query = readString("Search (Crop/Field)").toLowerCase();
        records.stream()
            .filter(r -> r.cropName.toLowerCase().contains(query) || r.fieldName.toLowerCase().contains(query))
            .forEach(System.out::println);
    }

    void editRecord() {
        if (records.isEmpty()) { noDataMsg(); return; }
        int id = readInt("Enter ID to edit", 1, 9999);
        CropRecord r = records.stream().filter(rec -> rec.id == id).findFirst().orElse(null);
        if (r == null) { System.out.println(C.RED + "  Not found." + C.RESET); return; }

        String newCrop = readOptional("New crop [" + r.cropName + "]");
        if (!newCrop.isEmpty()) r.cropName = newCrop;
        
        r.irrigationMm = readDoubleOptional("New irrigation [" + r.irrigationMm + "]", r.irrigationMm);
        System.out.println(C.GREEN + "  ✅  Updated." + C.RESET);
    }

    void deleteRecord() {
        int id = readInt("ID to delete", 1, 9999);
        if (records.removeIf(r -> r.id == id)) System.out.println(C.GREEN + "  ✅  Deleted." + C.RESET);
        else System.out.println(C.RED + "  ❌  Not found." + C.RESET);
    }

    void showSummary() {
        if (records.isEmpty()) { noDataMsg(); return; }
        double totalArea = records.stream().mapToDouble(r -> r.areaHectares).sum();
        double totalWater = records.stream().mapToDouble(CropRecord::irrigationVolumeLiters).sum();
        
        System.out.println("\n" + C.PURPLE + C.BOLD + "  📊  FARM SUMMARY" + C.RESET);
        System.out.printf("  Managed Area: %.2f ha%n", totalArea);
        System.out.printf("  Total Water:  %.0f Liters%n", totalWater);
    }

    // --- HELPERS ---
    String readString(String p) {
        System.out.print(C.CYAN + "  → " + p + ": " + C.RESET);
        return sc.nextLine().trim();
    }

    String readOptional(String p) {
        System.out.print(C.CYAN + "  → " + p + ": " + C.RESET);
        return sc.nextLine().trim();
    }

    int readInt(String prompt, int min, int max) {
        while (true) {
            System.out.print(C.CYAN + "  → " + prompt + ": " + C.RESET);
            try {
                int v = Integer.parseInt(sc.nextLine().trim());
                if (v >= min && v <= max) return v;
            } catch (Exception e) {}
            System.out.println(C.YELLOW + "    ⚠  Invalid input." + C.RESET);
        }
    }

    double readDouble(String prompt, double min, double max) {
        while (true) {
            System.out.print(C.CYAN + "  → " + prompt + ": " + C.RESET);
            try {
                return Double.parseDouble(sc.nextLine().trim());
            } catch (Exception e) {}
            System.out.println(C.YELLOW + "    ⚠  Invalid number." + C.RESET);
        }
    }

    double readDoubleOptional(String prompt, double current) {
        System.out.print(C.CYAN + "  → " + prompt + ": " + C.RESET);
        String in = sc.nextLine().trim();
        try { return in.isEmpty() ? current : Double.parseDouble(in); } 
        catch (Exception e) { return current; }
    }

    void printBanner() {
        System.out.println("\n" + C.BOLD + C.GREEN + "  🚜  FARM MANAGEMENT v3.0" + C.RESET);
    }

    void printMenu() {
        System.out.println("\n" + C.BOLD + "  [1] Add [2] View [3] Search [4] Summary [5] Edit [6] Delete [7] Export [0] Exit" + C.RESET);
    }

    void noDataMsg() { System.out.println(C.YELLOW + "  No records found." + C.RESET); }
}
