const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const gatewayIPmapSchema = new Schema(
  {
    GatewayID:{ type: String },
    IP:{ type: String }    
  },
  {
    timestamps: true,
    toJSON: { getters: true },
  }
);

module.exports = mongoose.model("GatewayIPmap", gatewayIPmapSchema, "gatewayIPmap");