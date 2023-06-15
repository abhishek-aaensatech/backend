const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const gatewaySchema = new Schema(
  {
    GatewayId:{ type: String },
    OptimizerIds:{ type : Array , "default" : []}
  },
  {
    timestamps: true,
    toJSON: { getters: true },
  }
);

module.exports = mongoose.model("GatewayIDs", gatewaySchema, "gatewayIDs");