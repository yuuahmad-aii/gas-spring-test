import pandas as pd

# Baca file CSV
input_file = "data_saya.csv"
output_file = "hasil.csv"

# Membaca data dari CSV
data = pd.read_csv(input_file)

# Filter data saat "Graph 3" bernilai 255
filtered_data = data[data["Graph 2"] == 255.0]

# Mencari nilai tertinggi dari "Graph 1" dan nilai "kg" terkait
if not filtered_data.empty:
    max_graph1 = filtered_data["Graph 1"].max()
    corresponding_kg = filtered_data.loc[
        filtered_data["Graph 1"] == max_graph1, "kg"
    ].values[0]
else:
    max_graph1 = None
    corresponding_kg = None

# Simpan hasil ke dalam DataFrame
result = pd.DataFrame(
    {"Max Graph 1": [max_graph1], "Corresponding kg": [corresponding_kg]}
)

# Simpan ke file CSV
result.to_csv(output_file, index=False)

print(f"Hasil telah disimpan ke file {output_file}")
