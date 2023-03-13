`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/12/2023 02:45:51 PM
// Design Name: 
// Module Name: gain_controller_stream
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module gain_controller_stream #
(
    parameter DATA_WIDTH = 32
)

(
    input s_axi_aclk,
    input s_axi_aresetn,
    
    input [DATA_WIDTH-1:0] s_axis_tdata,
    input s_axis_tlast,
    output s_axis_tready,
    input s_axis_tvalid,
    output reg [DATA_WIDTH-1:0] m_axis_tdata,
    input m_axis_tready,
    output reg m_axis_tvalid,
    output reg m_axis_tlast,
    input [DATA_WIDTH-1:0] gain_config
);
integer i;
reg sig = 1'b0;
reg sig1 = 1'b0;
reg [DATA_WIDTH-1:0] my;
reg [63:0] out = 64'd0;
reg [63:0] out1 = 64'd0;

assign s_axis_tready = m_axis_tready;

initial  begin
   for(i=0;i<DATA_WIDTH;i=i+1)
   begin
    m_axis_tdata[i] = 1'b0;
    my[i] = 1'b0;
   end 
   m_axis_tvalid = 1'b0;
   m_axis_tlast = 1'b0;
   
end
    
always @(posedge s_axi_aclk or negedge s_axi_aresetn)
begin
    if(!s_axi_aresetn)
    begin
        my <= 0;
        m_axis_tdata <= 0;
        out <= 64'd0;
        out1 <= 64'd0;
    end
    else begin
       if(s_axis_tvalid & s_axis_tready)
       begin
           my <= s_axis_tdata;
//           for(i=0;i<DATA_WIDTH/8;i=i+1)
//           begin
//            my[i*8+:8] <= s_axis_tdata[i*8+:8]; 
//           end 
           
           out <= (((my[14:0] << 8) * gain_config[23:0]) >> 16);
           sig <= my[15];
           out1 <= (((my[30:16] << 8) * gain_config[23:0]) >> 16);
           sig1 <= my[31];
           
           m_axis_tdata <= {sig1,out1[14:0],sig,out[14:0]};
      end
  end  
end

always @(posedge s_axi_aclk or negedge s_axi_aresetn)
begin
    if(!s_axi_aresetn)
    begin
        m_axis_tvalid <= 1'b0;
        m_axis_tlast <= 1'b0;
    end
    else begin
        m_axis_tvalid <= s_axis_tvalid;
        m_axis_tlast <= s_axis_tlast;
    end
end




endmodule
