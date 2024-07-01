# %% 

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.pipeline import make_pipeline


file_name = 'logs/2024_06_13_flight_arena/flight/log_8_2024-6-13-10-43-30.csv'

# Enable interactive plots
%matplotlib widget

# Load files
actuator_df = pd.read_csv('logs/2024_06_18_flying_calibration/actuator.csv')
current_df = pd.read_csv('logs/2024_06_18_flying_calibration/current.csv')

# Remove rows where the value is 1000 from the actuator DataFrame
actuator_df_cleaned = actuator_df[actuator_df['value'] != 1000]

# Convert the elapsed time to datetime format (if needed)
actuator_df_cleaned['elapsed time'] = pd.to_datetime(actuator_df_cleaned['elapsed time'], unit='s')
current_df['elapsed time'] = pd.to_datetime(current_df['elapsed time'], unit='s')

# Merge the two DataFrames based on the closest timestamps
merged_df = pd.merge_asof(actuator_df_cleaned.sort_values('elapsed time'), 
                          current_df.sort_values('elapsed time'), 
                          on='elapsed time', 
                          direction='nearest',
                          suffixes=('_actuator', '_current'))

# Extract features and target variables
X = merged_df['value_actuator'].values.reshape(-1, 1)
y = merged_df['value_current'].values

# Function to fit a polynomial regression model and plot results
def fit_and_plot_polynomial(X, y, degree):
    polyreg = make_pipeline(PolynomialFeatures(degree), LinearRegression())
    polyreg.fit(X, y)
    
    # Create a linspace from x_min to x_max for smooth plotting
    X_linspace = np.linspace(X.min(), X.max(), 500).reshape(-1, 1)
    y_pred = polyreg.predict(X_linspace)
    
    # Calculate metrics using original X and y
    y_pred_original = polyreg.predict(X)
    mse = mean_squared_error(y, y_pred_original)
    r2 = r2_score(y, y_pred_original)
    
    # Plot the data and the fitted curve
    plt.figure(figsize=(7, 5))
    plt.scatter(X, y, alpha=0.5, label='Data')
    plt.plot(X_linspace, y_pred, color='red', label=f'Polynomial Degree {degree}')
    plt.title(f'Polynomial Regression Degree {degree}')
    plt.xlabel('Actuator Output Value')
    plt.ylabel('Current Consumption (A)')
    plt.legend()
    plt.grid(True)
    plt.show()
    
    print(f"Degree {degree} Polynomial Regression:")
    print(f"Mean Squared Error: {mse}")
    print(f"R^2 Score: {r2}\n")

# Fit and plot polynomial regressions of degree 2 and 4
fit_and_plot_polynomial(X, y, 2)
fit_and_plot_polynomial(X, y, 4)

# %%

import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV file
motor_outputs_df = pd.read_csv(file_name)

# Pivot the DataFrame to have actuator outputs as columns
motor_outputs_pivoted_df = motor_outputs_df.pivot(index='elapsed time', columns='topic', values='value').reset_index()

motor_outputs_pivoted_df.replace(1500, 1000, inplace=True)


# Identify and replace isolated 1000s with NaN for interpolation
for col in ['actuator_outputs.output[0]', 'actuator_outputs.output[1]', 'actuator_outputs.output[2]', 'actuator_outputs.output[3]']:
    condition = (motor_outputs_pivoted_df[col] == 1000) & \
                (motor_outputs_pivoted_df[col].shift(1) != 1000) & \
                (motor_outputs_pivoted_df[col].shift(-1) != 1000)
    motor_outputs_pivoted_df.loc[condition, col] = np.nan

# Interpolate to fill NaN values
motor_outputs_pivoted_df.interpolate(inplace=True)


# Remove initial and final rows where all outputs are 1000
valid_start_idx = motor_outputs_pivoted_df[
    (motor_outputs_pivoted_df['actuator_outputs.output[0]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[1]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[2]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[3]'] != 1000)
].index[0]

valid_end_idx = motor_outputs_pivoted_df[
    (motor_outputs_pivoted_df['actuator_outputs.output[0]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[1]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[2]'] != 1000) |
    (motor_outputs_pivoted_df['actuator_outputs.output[3]'] != 1000)
].index[-1]

motor_outputs_pivoted_df = motor_outputs_pivoted_df.loc[valid_start_idx:valid_end_idx].reset_index(drop=True)


# Plot motor outputs
def plot_motor_outputs(df):
    fig, axs = plt.subplots(4, 1, figsize=(7, 6), sharex=True)
    
    fig.suptitle(f'Motor Outputs from {file_name}')
    
    axs[0].plot(df['elapsed time'], df['actuator_outputs.output[0]'], label='Output[0]', color='b')
    axs[0].set_ylabel('Output[0]')
    axs[0].legend()
    axs[0].grid(True)
    
    axs[1].plot(df['elapsed time'], df['actuator_outputs.output[1]'], label='Output[1]', color='g')
    axs[1].set_ylabel('Output[1]')
    axs[1].legend()
    axs[1].grid(True)
    
    axs[2].plot(df['elapsed time'], df['actuator_outputs.output[2]'], label='Output[2]', color='r')
    axs[2].set_ylabel('Output[2]')
    axs[2].legend()
    axs[2].grid(True)
    
    axs[3].plot(df['elapsed time'], df['actuator_outputs.output[3]'], label='Output[3]', color='c')
    axs[3].set_ylabel('Output[3]')
    axs[3].set_xlabel('Elapsed Time')
    axs[3].legend()
    axs[3].grid(True)
    
    plt.show()

plot_motor_outputs(motor_outputs_pivoted_df)



import numpy as np
import matplotlib.pyplot as plt

# Fit the polynomial regression model using the calibration data
degree = 4
polyreg = make_pipeline(PolynomialFeatures(degree), LinearRegression())
polyreg.fit(X, y)

# Predict the current consumption for each motor output
for col in ['actuator_outputs.output[0]', 'actuator_outputs.output[1]', 'actuator_outputs.output[2]', 'actuator_outputs.output[3]']:
    motor_outputs_pivoted_df[col + '_current'] = polyreg.predict(motor_outputs_pivoted_df[col].values.reshape(-1, 1))

# Calculate the summed current consumption
motor_outputs_pivoted_df['summed_current'] = (motor_outputs_pivoted_df['actuator_outputs.output[0]_current'] +
                                              motor_outputs_pivoted_df['actuator_outputs.output[1]_current'] +
                                              motor_outputs_pivoted_df['actuator_outputs.output[2]_current'] +
                                              motor_outputs_pivoted_df['actuator_outputs.output[3]_current'])

# Plot individual current consumption per motor and the summed current consumption
def plot_current_consumption(df):
    fig, axs = plt.subplots(5, 1, figsize=(7, 7), sharex=True)
    
    fig.suptitle('Motor Current Consumption and Summed Current Consumption')

    axs[0].plot(df['elapsed time'], df['actuator_outputs.output[0]_current'], label='Output[0] Current', color='b')
    axs[0].set_ylabel('Current (A)')
    axs[0].legend()
    axs[0].grid(True)

    axs[1].plot(df['elapsed time'], df['actuator_outputs.output[1]_current'], label='Output[1] Current', color='g')
    axs[1].set_ylabel('Current (A)')
    axs[1].legend()
    axs[1].grid(True)

    axs[2].plot(df['elapsed time'], df['actuator_outputs.output[2]_current'], label='Output[2] Current', color='r')
    axs[2].set_ylabel('Current (A)')
    axs[2].legend()
    axs[2].grid(True)

    axs[3].plot(df['elapsed time'], df['actuator_outputs.output[3]_current'], label='Output[3] Current', color='c')
    axs[3].set_ylabel('Current (A)')
    axs[3].legend()
    axs[3].grid(True)

    axs[4].plot(df['elapsed time'], df['summed_current'], label='Summed Current', color='m')
    axs[4].set_ylabel('Current (A)')
    axs[4].set_xlabel('Elapsed Time')
    axs[4].legend()
    axs[4].grid(True)
    
    plt.show()

plot_current_consumption(motor_outputs_pivoted_df)



# %% 

# Load the voltage data
voltage_file_name = file_name[:-4]+'_voltage.csv'
voltage_df = pd.read_csv(voltage_file_name)

# Convert the elapsed time to datetime format
motor_outputs_pivoted_df['elapsed time'] = pd.to_datetime(motor_outputs_pivoted_df['elapsed time'], unit='s')
voltage_df['elapsed time'] = pd.to_datetime(voltage_df['elapsed time'], unit='s')

# Resample the voltage data to match the motor output data
# Set 'elapsed time' as index
voltage_df.set_index('elapsed time', inplace=True)
motor_outputs_pivoted_df.set_index('elapsed time', inplace=True)

# Resample voltage data to match the frequency of motor outputs data
voltage_df_resampled = voltage_df.resample('1S').mean().interpolate()  # Adjust the resampling frequency as needed
motor_outputs_resampled_df = motor_outputs_pivoted_df.resample('1S').mean()

# Reset the index to turn 'elapsed time' back into a column
voltage_df_resampled.reset_index(inplace=True)
motor_outputs_resampled_df.reset_index(inplace=True)

# Merge the resampled voltage data with the motor outputs data
merged_flight_df = pd.merge_asof(motor_outputs_resampled_df.sort_values('elapsed time'), 
                                 voltage_df_resampled.sort_values('elapsed time'), 
                                 on='elapsed time', 
                                 direction='nearest')

# Calculate power consumption (Power = Current * Voltage)
merged_flight_df['power'] = merged_flight_df['summed_current'] * merged_flight_df['value']

# Plot voltage over time and power over time
def plot_voltage_and_power(df):
    fig, axs = plt.subplots(2, 1, figsize=(5, 6), sharex=True)
    
    fig.suptitle('Voltage and Power Consumption Over Time')

    axs[0].plot(df['elapsed time'], df['value'], label='Voltage (V)', color='b')
    axs[0].set_ylabel('Voltage (V)')
    axs[0].legend()
    axs[0].grid(True)

    axs[1].plot(df['elapsed time'], df['power'], label='Power Consumption (W)', color='r')
    axs[1].set_ylabel('Power (W)')
    axs[1].set_xlabel('Elapsed Time')
    axs[1].legend()
    axs[1].grid(True)
    
    plt.show()

plot_voltage_and_power(merged_flight_df)

# %% 


# Load the CSV file containing flight data
flight_data_file = 'logs/2024_06_28_flight_field/log_39_2024-6-28-12-00-42_long_flight.csv'
flight_df = pd.read_csv(flight_data_file)



# Pivot the flight data to create one column per topic
pivot_df = flight_df.pivot(index='elapsed time', columns='topic', values='value').reset_index()


# Analyse motor outputs
pivot_df.replace(1500, 1000, inplace=True)

for col in ['actuator_outputs.output[0]', 'actuator_outputs.output[1]', 'actuator_outputs.output[2]', 'actuator_outputs.output[3]']:
    condition = (pivot_df[col] == 1000) & \
                (pivot_df[col].shift(1) != 1000) & \
                (pivot_df[col].shift(-1) != 1000)
    pivot_df.loc[condition, col] = np.nan


# Remove initial and final rows where all outputs are 1000 or NaN
valid_start_idx = pivot_df[
    (pivot_df['actuator_outputs.output[0]'].notna()) & (pivot_df['actuator_outputs.output[0]'] != 1000) |
    (pivot_df['actuator_outputs.output[1]'].notna()) & (pivot_df['actuator_outputs.output[1]'] != 1000) |
    (pivot_df['actuator_outputs.output[2]'].notna()) & (pivot_df['actuator_outputs.output[2]'] != 1000) |
    (pivot_df['actuator_outputs.output[3]'].notna()) & (pivot_df['actuator_outputs.output[3]'] != 1000)
].index[0]

valid_end_idx = pivot_df[
    (pivot_df['actuator_outputs.output[0]'].notna()) & (pivot_df['actuator_outputs.output[0]'] != 1000) |
    (pivot_df['actuator_outputs.output[1]'].notna()) & (pivot_df['actuator_outputs.output[1]'] != 1000) |
    (pivot_df['actuator_outputs.output[2]'].notna()) & (pivot_df['actuator_outputs.output[2]'] != 1000) |
    (pivot_df['actuator_outputs.output[3]'].notna()) & (pivot_df['actuator_outputs.output[3]'] != 1000)
].index[-1]



pivot_df = pivot_df.loc[valid_start_idx:valid_end_idx].reset_index(drop=True)



# Set 'elapsed time' as the index for interpolation
pivot_df.set_index('elapsed time', inplace=True)

# Interpolate all fields in the DataFrame
pivot_df.interpolate(method='linear', inplace=True)

# Reset the index if needed
pivot_df.reset_index(inplace=True)


display(pivot_df)





# Fit the polynomial regression model using the calibration data
degree = 4
polyreg = make_pipeline(PolynomialFeatures(degree), LinearRegression())
polyreg.fit(X, y)

# Predict the current consumption for each motor output
for col in ['actuator_outputs.output[0]', 'actuator_outputs.output[1]', 'actuator_outputs.output[2]', 'actuator_outputs.output[3]']:
    pivot_df[col + '_current'] = polyreg.predict(pivot_df[col].values.reshape(-1, 1))

# Calculate the summed current consumption
pivot_df['summed_current'] = (pivot_df['actuator_outputs.output[0]_current'] +
                                              pivot_df['actuator_outputs.output[1]_current'] +
                                              pivot_df['actuator_outputs.output[2]_current'] +
                                              pivot_df['actuator_outputs.output[3]_current'])

# Verify that summed_current values are above 0
pivot_df['summed_current'] = pivot_df['summed_current'].clip(lower=0)



# Plot motor outputs
fig, axs = plt.subplots(4, 1, figsize=(8, 6), sharex=True)

fig.suptitle(f'Motor Outputs from {file_name}')

axs[0].plot(pivot_df['elapsed time'], pivot_df['actuator_outputs.output[0]_current'], label='Output[0]', color='b')
axs[0].set_ylabel('Output[0]_current')
axs[0].legend()
axs[0].grid(True)

axs[1].plot(pivot_df['elapsed time'], pivot_df['actuator_outputs.output[1]_current'], label='Output[1]', color='g')
axs[1].set_ylabel('Output[1]_current')
axs[1].legend()
axs[1].grid(True)

axs[2].plot(pivot_df['elapsed time'], pivot_df['actuator_outputs.output[2]_current'], label='Output[2]', color='r')
axs[2].set_ylabel('Output[2]_current')
axs[2].legend()
axs[2].grid(True)

axs[3].plot(pivot_df['elapsed time'], pivot_df['actuator_outputs.output[3]_current'], label='Output[3]', color='c')
axs[3].set_ylabel('Output[3]_current')
axs[3].set_xlabel('Elapsed Time')
axs[3].legend()
axs[3].grid(True)
    
plt.show()


# Plot the voltage column
plt.figure(figsize=(8, 6))
plt.plot(pivot_df['elapsed time'], pivot_df['battery_status.voltage_v'], label='Voltage (V)', color='b')
plt.xlabel('Elapsed Time')
plt.ylabel('Voltage (V)')
plt.title('Voltage Over Time')
plt.legend()
plt.grid(True)
plt.show()




# %% Calculate power, gnss, distance, velocity

# Calculate power consumption (Power = Voltage * Summed Current)
pivot_df['power'] = pivot_df['battery_status.voltage_v'] * pivot_df['summed_current']


# Plot the power column
plt.figure(figsize=(8, 6))
plt.plot(pivot_df['elapsed time'], pivot_df['power'], label='Power (W)', marker='o', markersize=2, color='purple')
plt.xlabel('Elapsed Time')
plt.ylabel('Power (W)')
plt.title('Power Over Time')
plt.legend()
plt.grid(True)
plt.show()

pivot_df['elapsed time'] = pd.to_datetime(pivot_df['elapsed time'], unit='s')

# Calculate the total energy consumption in Joules (since power is in watts and time in seconds)
pivot_df['energy'] = pivot_df['power'] * pivot_df['elapsed time'].diff().dt.total_seconds().fillna(0)

# Calculate cumulative energy
pivot_df['cumulative_energy'] = pivot_df['energy'].cumsum()





# Plot latitude, longitude, altitude, and cumulative distance
fig, axs = plt.subplots(3, 1, figsize=(8, 8), sharex=True)

fig.suptitle('GNSS Data')

axs[0].plot(pivot_df['elapsed time'], pivot_df['estimator_global_position.lat'], label='Latitude', color='b')
axs[0].set_ylabel('Latitude')
axs[0].legend()
axs[0].grid(True)

axs[1].plot(pivot_df['elapsed time'], pivot_df['estimator_global_position.lon'], label='Longitude', color='g')
axs[1].set_ylabel('Longitude')
axs[1].legend()
axs[1].grid(True)

axs[2].plot(pivot_df['elapsed time'], pivot_df['estimator_global_position.alt'], label='Altitude', color='r')
axs[2].set_ylabel('Altitude')
axs[2].legend()
axs[2].grid(True)

plt.show()




from geopy.distance import geodesic

# Calculate cumulative distance using latitude and longitude data, excluding rows where power is zero
def calculate_cumulative_distance(df):
    distances = [0]  # Start with a cumulative distance of 0
    cumulative_distance = 0
    for i in range(1, len(df)):
        if df['power'].iloc[i] > 0 and pd.notna(df['estimator_global_position.lat'].iloc[i-1]) and pd.notna(df['estimator_global_position.lon'].iloc[i-1]) and \
           pd.notna(df['estimator_global_position.lat'].iloc[i]) and pd.notna(df['estimator_global_position.lon'].iloc[i]):
            
            coord1 = (df['estimator_global_position.lat'].iloc[i-1], df['estimator_global_position.lon'].iloc[i-1])
            coord2 = (df['estimator_global_position.lat'].iloc[i], df['estimator_global_position.lon'].iloc[i])
            
            # Calculate distance in meters
            distance = geodesic(coord1, coord2).meters
            cumulative_distance += distance
        
        distances.append(cumulative_distance)
    
    df['cumulative_distance'] = distances[:len(df)]



# Calculate cumulative distance
calculate_cumulative_distance(pivot_df)




# Plot cumulative distance and cumulative energy
fig, ax1 = plt.subplots(figsize=(8, 6))

ax1.set_xlabel('Elapsed Time')
ax1.set_ylabel('Cumulative Distance (m)', color='tab:blue')
ax1.plot(pivot_df['elapsed time'], pivot_df['cumulative_distance'], color='tab:blue', marker='o', markersize=2, label='Cumulative Distance')
ax1.tick_params(axis='y', labelcolor='tab:blue')

ax2 = ax1.twinx()
ax2.set_ylabel('Cumulative Energy (J)', color='tab:red')
ax2.plot(pivot_df['elapsed time'], pivot_df['cumulative_energy'], color='tab:red', marker='o', markersize=2, label='Cumulative Energy')
ax2.tick_params(axis='y', labelcolor='tab:red')

fig.tight_layout()
plt.title('Cumulative Distance and Cumulative Energy')
fig.legend(loc='upper left', bbox_to_anchor=(0.1,0.9))
plt.grid(True)
plt.show()



mass_of_robot = 3.5  # in kilograms
gravity = 9.81  # in m/s^2
total_energy = pivot_df['cumulative_energy'].iloc[-1]  # in Joules
total_distance = pivot_df['cumulative_distance'].iloc[-1]  # in meters


cost_of_transport = total_energy / (mass_of_robot * gravity * total_distance)

print(f"Total Energy Consumption (J): {total_energy}")
print(f"Total Distance Traveled (m): {total_distance}")
print(f"Cost of Transport (dimensionless): {cost_of_transport}")

# %%

# This is the expected Cost of Transport for the speed flown (approx. 5m/s)
# The drone might be able to fly up to 20m/s reducing the Cost of Transport

