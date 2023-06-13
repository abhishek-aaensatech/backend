const express = require("express");
const cors = require("cors")
require('dotenv').config();
const DbConnect = require('./database.js');
const DetailsModel = require("./models/details.js");
const bodyParser = require('body-parser');

DbConnect();

const PORT = process.env.PORT || 5000;
const app = express();

app.use(cors())  
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())

 
app.get('/',(req,res)=>{
    return res.send("Server started");
})

app.get("/logs",async (req,res)=>{
    const allDetails = await DetailsModel.find({});
    if(!allDetails)
        return res.status(404).json({});
    return res.status(200).json(allDetails);
})

app.post("/getLatestData",async (req,res)=>{
    const {GatewayId,OptimizerId} = req.body;
    if((!GatewayId) || (!OptimizerId))
        return res.status(400).json({message: "Either GatewayID or OptimizerId not available"});
    const val = await DetailsModel.find({GatewayId,OptimizerId}).sort({"created_at": -1});
    if(!val)
        return res.status(404).json({message:"No Data Found for given GatewayID and OptimizerId"});
    console.log(val);
    return res.status(200).json(val[0]);
})

app.post('/sendNewDetails',async (req,res)=>{
   const detail = req.body;
   const val = await DetailsModel.create(detail);
   console.log(val); 
   return res.status(200).json(val);
}) 

app.listen(PORT, ()=>{
    console.log("Server started on PORT ",PORT);
})