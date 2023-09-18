const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const ToggleSchema = new Schema(
  {
    GatewayID:{ type: String },
    OptimizerID:{ type: String },
    Flag: {type: Boolean}
  }
);

module.exports = mongoose.model("ToggleModel", ToggleSchema, "ToggleModel");