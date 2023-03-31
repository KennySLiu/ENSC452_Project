
module debounce_HIGH_LOW (
  clk,
  reset_n,
  data_in,
  data_out
);

  parameter WIDTH = 8;           // set to be the width of the bus being debounced
  parameter TIMEOUT = 50000;      // 1us
  parameter TIMEOUT_WIDTH = 16;   // set to be ceil(log2(TIMEOUT))
  
  input wire clk;
  input wire reset_n;
  
  input wire [WIDTH-1:0] data_in;
  output reg [WIDTH-1:0] data_out;
  
  reg [TIMEOUT_WIDTH-1:0] counter [0:WIDTH-1];
  reg [WIDTH-1:0] data_p;
  reg [WIDTH-1:0] out_check;
  reg counter_reset [0:WIDTH-1];
  reg counter_enable [0:WIDTH-1];
  reg [7:0] j = 8'b0;
  
  initial begin
	for(j=0; j < WIDTH; j=j+1) begin
		data_out[j] = 0;
		data_p[j] = 0;
		out_check[j] = 0;
		counter_reset[j] = 1;
		counter_enable[j] = 0;
	end
  end
  
  // need one counter per input to debounce
  genvar i;
  generate for (i = 0; i < WIDTH; i = i+1)
  begin:  debounce_counter_loop
	always @ (posedge clk or negedge reset_n)
    begin
      if (reset_n == 0)
      begin
        counter[i] <= 0;
      end
      else
      begin
        if (counter_reset[i] == 1)  // resetting the counter needs to win
        begin
          counter[i] <= 0;
        end
        else if (counter_enable[i] == 1)
        begin
          counter[i] <= counter[i] + 1'b1;
        end
      end
    end
  
  	always @ (posedge clk or negedge reset_n)
    begin
		if (reset_n == 0)
		  begin
			counter_reset[i] <= 1;
			counter_enable[i] <= 0;
			out_check[i] <= 0;
			data_p[i] = 0;
			data_out[i] = 0;
		  end
		else if (data_p[i] != data_in[i])
		begin
		  data_p[i] <= data_in[i];
		  counter_reset[i] <= 1;
		  out_check[i] <= 1;
		  
		end
		else if(out_check[i] & (data_p[i] == data_in[i]))
		begin
			data_p[i] <= data_in[i];
			counter_reset[i] <= 0;
			counter_enable[i] <= 1;
			if(counter[i] >= TIMEOUT)
			begin 
				data_out[i] <= data_p[i];
				out_check[i] <= 0;
			end
			else begin
				data_out[i] <= data_out[i];
			end
			
		end
		else
		begin
		  counter_reset[i] <= 1;
		  counter_enable[i] <= 0;
		end
	end 


	  
  end  
  endgenerate
  
endmodule
