Farm & Irrigation Tools
A collection of practical agricultural software tools built with HTML, C++, and Java to help farmers and agronomists manage crop water needs, irrigation flow rates, and farm data.

Projects

1.Crop Water Calculator (HTML/CSS/JS)
An interactive browser-based calculator that estimates daily and seasonal irrigation water requirements for common crops.
Features:
Select from 8 crop types (Wheat, Rice, Maize, Cotton, Sugarcane, etc.)

Adjusts for growth stage (Initial → Mid-Season → Late)

Inputs for ET₀, effective rainfall, and soil type

Outputs: Crop coefficient (Kc), ETc, net water need, daily volume (m³), seasonal total, and irrigation frequency


2.Irrigation Flow Rate Calculator (C++)
A console application that calculates irrigation flow rates using standard hydraulic engineering formulas.

Calculation Modules:
| Module | Formula Used |
|---|---|
| Open-Channel Flow | Manning's: Q = (1/n) × A × R^(2/3) × S^(1/2) |
| Drip Emitter Discharge | q = Kd × H^x |
| Field Water Demand | V = (ETc - Pe) × A / Ea |

3.Farm Data Input App (Java)
A full-featured console application for recording, managing, and exporting farm field data.

Features:
Add crop records (crop, field, area, soil, irrigation, fertilizer)

📄 View all records in a formatted table

🔍 Search by crop or field name

Farm summary with totals, averages & crop breakdown

✏️ Edit existing records

💾 Export to CSV with auto-dated 

📁 File Structurefarm-irrigation-tools/
├── crop_water_calculator.html   # Web-based crop water calculator
├── irrigation_flow_rate.cpp     # C++ irrigation flow calculator
├── FarmDataApp.java             # Java farm data input app
└── README.md

Use Cases
Small and medium-scale farmers estimating daily water needs
Agronomists calculating irrigation system flow rates
Farm managers recording and tracking field data across seasons

👤 Author
[Himanshu Prasad Hankara]
LinkedIn 
https://www.linkedin.com/in/himanshu-prasad-hankara-52a237384?utm_source=share&utm_campaign=share_via&utm_content=profile&utm_medium=android_app

GitHub
https://github.com/H2P-Made/Farm-Irrigation-Tools-
