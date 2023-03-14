`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/12/2023 06:42:16 PM
// Design Name: 
// Module Name: mul_calc
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


module mul_calc #
(
    parameter ENRGY_DATA_WIDTH = 40,
    parameter AXI_DATA_WIDTH = 32,
    parameter NUM_FFT_PTS  = 16,
    parameter Thresh = 1000,
    parameter Ratio = 10
)

(
    input aclk,
    input aresetn,
    //input [31:0] Thresh,
    //input [31:0] Ratio,
    
    input [ENRGY_DATA_WIDTH-1 : 0] energy,
    input e_ready,
    
    output reg [AXI_DATA_WIDTH-1:0] m_axis_a_tdata,
    input m_axis_a_tready,
    output reg m_axis_a_tvalid,
    
    output reg [AXI_DATA_WIDTH-1:0] m_axis_b_tdata,
    input m_axis_b_tready,
    output reg m_axis_b_tvalid
);

reg [7:0] state = 8'd0;
reg [7:0] state_wait = 8'd0;
reg [31:0] counter = 32'd0;
reg done = 1'b0;
reg [ENRGY_DATA_WIDTH-1 : 0] data_reg;
reg [63:0] data_sum = 64'd0;


localparam IDLE = 8'd0;
localparam SUM = 8'd1;

localparam D_READY = 8'd0;
localparam WAIT_READY = 8'd1;


integer i;

initial begin
    for(i = 0; i < ENRGY_DATA_WIDTH; i = i+1)
    begin
        data_reg[i] = 1'b0;
    end
    for(i = 0; i < AXI_DATA_WIDTH; i = i+1)
    begin
        m_axis_a_tdata[i] = 1'b0;
        m_axis_b_tdata[i] = 1'b0;
    end
    
    m_axis_a_tvalid = 0;
    m_axis_b_tvalid = 0;
end


always  @ (posedge  aclk or negedge  aresetn)
begin
   if(!aresetn)
   begin
    state <= IDLE;
   
   end
   else begin
    case(state)
        IDLE:
            begin
                done <= 0;
                if(e_ready)
                begin
                    state <= SUM;
                    data_reg <= energy;
                end
                else begin
                    data_reg <= data_reg;
                    state <= IDLE;
                end
            
            end 
        
        SUM:
            begin
                state <= IDLE;
                if(counter >= (NUM_FFT_PTS-1))
                begin
                    counter <= 0;
                    m_axis_a_tdata <= (((data_sum+data_reg)-Thresh)/Ratio)+Thresh;
                    m_axis_b_tdata <= (data_sum+data_reg);
                    done <= 1;
                    data_sum <= 64'd0;
                end
                else begin
                    data_sum <= data_sum + data_reg;
                    counter = counter + 1;
                end
            end
        
        default:
        begin
            state <= IDLE;
        end
    endcase
   
   end
   

end

always  @ (posedge  aclk or negedge  aresetn)
begin
   if(!aresetn)
   begin
   
	state_wait <= D_READY;
	m_axis_a_tvalid <= 1'b0;
	m_axis_b_tvalid <= 1'b0;
   
   end
   else begin
	case(state_wait)
		D_READY:
		begin
			m_axis_a_tvalid <= 1'b0;
			m_axis_b_tvalid <= 1'b0;
			
			if(done)
			begin 
				if(m_axis_a_tready & m_axis_b_tready)
				begin
					m_axis_a_tvalid <= 1'b1;
					m_axis_b_tvalid <= 1'b1;
					state_wait <= D_READY;
				end
				else begin
					state_wait <= WAIT_READY;
				end
			end
			else begin 
				state_wait <= D_READY; 
			end
		end
		
		WAIT_READY:
		begin
			if(m_axis_a_tready & m_axis_b_tready)
			begin
				m_axis_a_tvalid <= 1'b1;
				m_axis_b_tvalid <= 1'b1;
				state_wait <= D_READY;
			end
			else begin
				state_wait <= WAIT_READY;
			end
		end
		
		
		default:
		begin
		  state_wait <= D_READY;
          m_axis_a_tvalid <= 1'b0;
          m_axis_b_tvalid <= 1'b0;
		end
		
	endcase
   
   end
   

end
  
endmodule
