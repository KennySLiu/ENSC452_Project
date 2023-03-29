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
    //input s_axis_tlast,
    output reg s_axis_tready,
    input s_axis_tvalid,
    output reg [DATA_WIDTH-1:0] m_axis_tdata,
    input m_axis_tready,
    output reg m_axis_tvalid,
    //output reg m_axis_tlast,
    input [DATA_WIDTH-1:0] gain_config
);
integer i;

initial  begin
   for(i=0;i<DATA_WIDTH;i=i+1)
   begin
    m_axis_tdata[i] = 1'b0;
   end 
   m_axis_tvalid = 1'b0;
   //m_axis_tlast = 1'b0;
   
end

reg l0 = 1'b0;
reg r0 = 1'b0;
reg v0 = 1'b0;

reg signed [47:0] re_result;
reg signed [47:0] img_result;

wire signed [23:0] real_P = {s_axis_tdata[15:0],8'd0};
wire signed [23:0] img_P = {s_axis_tdata[31:16],8'd0};
wire signed [23:0] gain_mul = gain_config[23:0];


always @(posedge s_axi_aclk )
begin
    if ( s_axi_aresetn == 1'b0 )
    begin
        //m_axis_tlast <= 1'b0;
    end
    else begin
        //l0 <= s_axis_tlast;
        //m_axis_tlast <= l0;
    end
end

always @(posedge s_axi_aclk )
begin
    if ( s_axi_aresetn == 1'b0 )
    begin
        m_axis_tvalid <= 1'b0;
    end
    else begin
        v0 <= s_axis_tvalid;
        //v1  <= v0;
        m_axis_tvalid <= v0;
    end
end

always @(posedge s_axi_aclk )
begin
    if ( s_axi_aresetn == 1'b0 )
    begin
        s_axis_tready <= 1'b0;
    end
    else begin
        r0 <= m_axis_tready;
        //r1  <= r0;
        s_axis_tready <= r0;
    end
end

    
always @(posedge s_axi_aclk or negedge s_axi_aresetn)
begin
    if(!s_axi_aresetn)
    begin
        m_axis_tdata <= 0;
    end
    else begin
       if(s_axis_tvalid & m_axis_tready)
       begin
			re_result <= (real_P*gain_mul);
            img_result <= (img_P*gain_mul); 
       end
	  
	  if(v0 & r0)
	  begin 
		m_axis_tdata[15:0] <= re_result[31:16];
		m_axis_tdata[31:16] <= img_result[31:16];
	  end
  end  
end

endmodule
