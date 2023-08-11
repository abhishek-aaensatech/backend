const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const detailsSchema = new Schema(
  {
    Date:{ type: String },
    Time:{ type: String },
    OptimizerID:{ type: String },
    GatewayID:{ type: String },
    OptimizerMode:{ type: String },
    RoomTemperature:{ type: Number },
    CoilTemperature:{ type: Number },
    "Ph1Voltage(V)":{ type: String },
    "Humidity(%)":{ type: Number },
    "Ph1Voltage(V)":{ type: String },
    "Ph1Current(A)":{ type: String },
    "Ph1ActivePower(kW)":{ type: String },
    "Ph1PowerFactor":{ type: String },
    "Ph1ApparentPower(kVA)":{ type: String },
    "Ph2Voltage(V)":{ type: String },
    "Ph2Current(A)":{ type: String },
    "Ph2ActivePower(kW)": { type: String },
    Ph2PowerFactor: { type: String},
    "Ph2ApparentPower(kVA)": { type : String },
    "Ph3Voltage(V)": { type : String },
    "Ph3Current(A)": { type : String },
    "Ph3ActivePower(kW)": { type : String },
    Ph3PowerFactor: { type : String },
    "Ph2ApparentPower(kVA)": { type : String },
  },
  {
    timestamps: true,
    toJSON: { getters: true },
  }
);

module.exports = mongoose.model("Details", detailsSchema, "details");